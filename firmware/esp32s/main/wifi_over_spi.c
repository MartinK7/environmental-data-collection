
#include "wifi_over_spi.h"

#include <string.h>
#include <assert.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/spi_slave.h"
#include "driver/gpio.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"

#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/sys.h"

static const char *TAG = "wifi_over_spi";

#define _LO(x)    ((x) & 0x00FFU)
#define _HI(x)   (((x) & 0xFF00U) >> 8U)
#define _LOHI(x) ((uint16_t)(x[0]) | ((uint16_t)(x[1]) << (uint16_t)8))

/// ####################################################################################################################
/// SPI registers
/// ####################################################################################################################

// Event group
static EventGroupHandle_t wos_event_group;
static struct wifi_over_spi_registers registers = {0}; // __attribute__((packed, aligned(4)))

/// ####################################################################################################################
/// LED Blink
/// ####################################################################################################################

#define GPIO_LED      2

static void led_task(void *pvParameters)
{
	gpio_reset_pin(GPIO_LED);
	gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);

	static uint8_t state = 0;

	while (1) {
		gpio_set_level(GPIO_LED, state);
		state = !state;
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

/// ####################################################################################################################
/// TCP Client
/// ####################################################################################################################

static void tcp_client_task(void *pvParameters)
{
	int err = 0;

	while (1) {
		if(xEventGroupGetBits(wos_event_group) & ESP32_REGISTER_ADDRESS_STATUS_TCP_FAIL_BIT) {
			ESP_LOGE(TAG, "TCP connection failed!");
		}
		
		if(xEventGroupGetBits(wos_event_group) & ESP32_REGISTER_ADDRESS_STATUS_TCP_CONNECTED_BIT) {
			xEventGroupSetBits(wos_event_group, ESP32_REGISTER_ADDRESS_STATUS_TCP_DISCONNECTED_BIT);
			ESP_LOGI(TAG, "TCP connection ended");
		}

		// Clear state machine
		xEventGroupClearBits(wos_event_group,
				ESP32_REGISTER_ADDRESS_CONTROL_TCP_SEND_REQUEST_BIT |
				ESP32_REGISTER_ADDRESS_CONTROL_TCP_DISCONNECT_REQUEST_BIT |
				ESP32_REGISTER_ADDRESS_STATUS_TCP_CONNECTED_BIT |
				ESP32_REGISTER_ADDRESS_STATUS_TCP_SENT_BIT);

		// Wait for request
		xEventGroupWaitBits(wos_event_group,
				ESP32_REGISTER_ADDRESS_CONTROL_TCP_CONNECT_REQUEST_BIT |
				ESP32_REGISTER_ADDRESS_STATUS_WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

		// Clear request and result bits
		xEventGroupClearBits(wos_event_group,
				ESP32_REGISTER_ADDRESS_CONTROL_TCP_CONNECT_REQUEST_BIT |
				ESP32_REGISTER_ADDRESS_STATUS_TCP_DISCONNECTED_BIT |
				ESP32_REGISTER_ADDRESS_STATUS_TCP_FAIL_BIT);
	
		// Get IP first by domain name
		ip_addr_t resolved_ip;
		for(int retry = 0; retry < 10; ++retry) {
			// Make sure it is terminated!
			registers.tcp_domain[sizeof(registers.tcp_domain) - 1] = '\0';
			// Info
			ESP_LOGI(TAG, "dns_gethostbyname '%s'", (const char*)registers.tcp_domain);
			// Try to resolve IP
			err = dns_gethostbyname((const char*)registers.tcp_domain, &resolved_ip, NULL, NULL);
			if(err == ERR_OK) {
				break;
			}
			ESP_LOGW(TAG, "dns_gethostbyname error: errno %d", err);
			vTaskDelay(5000 / portTICK_PERIOD_MS);
		}
		
		// Error obtaining IP via DNS
		if(err != ERR_OK) {
			xEventGroupSetBits(wos_event_group, ESP32_REGISTER_ADDRESS_STATUS_TCP_FAIL_BIT);
			continue;
		}
		
		ESP_LOGI(TAG, "dns_gethostbyname OK!");

		// Convert ip_addr_t --> u32 in_addr_t
		struct in_addr ip_addr;
		ip_addr.s_addr = ip4_addr_get_u32(&resolved_ip.u_addr.ip4);
		
		// Configure TCP connection
		struct sockaddr_in dest_addr;
		memset(&dest_addr, 0, sizeof(dest_addr));
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_addr = ip_addr;
		dest_addr.sin_port = htons(_LOHI(registers.tcp_port));
	
		// Create socket
		int sock;
		for(int retry = 0; retry < 10; ++retry) {
			sock = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
			if(sock >= 0) {
				break;
			}
			ESP_LOGW(TAG, "Unable to create socket: errno %d", errno);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}
		
		// Error creating socket
		if(sock < 0) {
			xEventGroupSetBits(wos_event_group, ESP32_REGISTER_ADDRESS_STATUS_TCP_FAIL_BIT);
			continue;
		}
		
		// Print debug message
		char ip_str[32];
		inet_ntoa_r(ip_addr, ip_str, sizeof(ip_str));	
		ESP_LOGI(TAG, "Socket %d created, connecting to %s:%d", sock, ip_str, ntohs(dest_addr.sin_port));

		// Connect socket to server
		for(int retry = 0; retry < 10; ++retry) {
			err = lwip_connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
			if(err == 0) {
				break;
			}
			ESP_LOGW(TAG, "Socket unable to connect: errno %d", errno);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}
		
		// Error cannot connect
		if(err != 0) {
			// Free socket
			lwip_shutdown(sock, 0);
			lwip_close(sock);
			xEventGroupSetBits(wos_event_group, ESP32_REGISTER_ADDRESS_STATUS_TCP_FAIL_BIT);
			continue;
		}
		
		// Success
		xEventGroupSetBits(wos_event_group, ESP32_REGISTER_ADDRESS_STATUS_TCP_CONNECTED_BIT);
		ESP_LOGI(TAG, "Successfully connected");

		
		TimeOut_t xTimeOut;
		TickType_t xTicksToWait = 10000 / portTICK_PERIOD_MS;
		// Reset timeout
		vTaskSetTimeOutState(&xTimeOut);
		// Do for every transmit
		while(!(xEventGroupGetBits(wos_event_group) & ESP32_REGISTER_ADDRESS_CONTROL_TCP_DISCONNECT_REQUEST_BIT)) {
			if(xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) != pdFALSE) {
				// Timeout
				ESP_LOGE(TAG, "Send request timeout!");
				xEventGroupSetBits(wos_event_group, ESP32_REGISTER_ADDRESS_STATUS_TCP_FAIL_BIT);
				break;
			}
			// Do for every request
			if(xEventGroupWaitBits(wos_event_group, ESP32_REGISTER_ADDRESS_CONTROL_TCP_SEND_REQUEST_BIT | ESP32_REGISTER_ADDRESS_CONTROL_TCP_DISCONNECT_REQUEST_BIT, pdFALSE, pdFALSE, xTicksToWait) & ESP32_REGISTER_ADDRESS_CONTROL_TCP_SEND_REQUEST_BIT) {
				xEventGroupClearBits(wos_event_group, ESP32_REGISTER_ADDRESS_STATUS_TCP_SENT_BIT);
				uint16_t cnt = _LOHI(registers.tcp_buffer_tx_len);
				ESP_LOGI(TAG, "Let's send 0x%04X == %d bytes ...", cnt, (int)cnt);
				err = lwip_send(sock, registers.tcp_buffer_tx, cnt, 0);
				if(err < 0) {
					continue;
				}
				xEventGroupSetBits(wos_event_group, ESP32_REGISTER_ADDRESS_STATUS_TCP_SENT_BIT);
				xEventGroupClearBits(wos_event_group, ESP32_REGISTER_ADDRESS_CONTROL_TCP_SEND_REQUEST_BIT);
				// Reset timeout
				vTaskSetTimeOutState(&xTimeOut);
				ESP_LOGI(TAG, "Data successfully sent!");
			}
		}

		// Free socket
		ESP_LOGI(TAG, "Shutting down socket and restarting...");
		lwip_shutdown(sock, 0);
		lwip_close(sock);
	}
	
	vTaskDelete(NULL);
}

/// ####################################################################################################################
/// WiFi station
/// ####################################################################################################################

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	static int s_retry_num = 0;
	
	if(event_base == WIFI_EVENT) {
		if(event_id == WIFI_EVENT_STA_START) {
			esp_wifi_connect();
			return;
		}
		
		if(event_id == WIFI_EVENT_STA_DISCONNECTED) {
			if(s_retry_num++ < 10) {
				esp_wifi_connect();
				ESP_LOGI(TAG, "retry to connect to the AP");
				return;
			}
			xEventGroupSetBits(wos_event_group, ESP32_REGISTER_ADDRESS_STATUS_WIFI_FAIL_BIT);
			ESP_LOGI(TAG,"connect to the AP fail");
			return;
		}
	}
	
	if(event_base == IP_EVENT) {
		if(event_id == IP_EVENT_STA_GOT_IP) {
			ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
			ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
			s_retry_num = 0;
			xEventGroupSetBits(wos_event_group, ESP32_REGISTER_ADDRESS_STATUS_WIFI_CONNECTED_BIT);
			return;
		}
	}
}

static void wifi_sta_task(void *pvParameters)
{		
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

	while(1) {
		// Wait for connection request
		xEventGroupWaitBits(wos_event_group, ESP32_REGISTER_ADDRESS_CONTROL_WIFI_CONNECT_REQUEST_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
		
		xEventGroupClearBits(wos_event_group,
				ESP32_REGISTER_ADDRESS_CONTROL_WIFI_CONNECT_REQUEST_BIT |
				ESP32_REGISTER_ADDRESS_STATUS_WIFI_FAIL_BIT |
				ESP32_REGISTER_ADDRESS_STATUS_WIFI_CONNECTED_BIT);
	
		// SSID and PASSWORD should be ready
		wifi_config_t wifi_config = {
			.sta = {
				.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
				.sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
			},
		};	
		
		// Copy SSID
		assert(sizeof(wifi_config.sta.ssid) == sizeof(registers.wifi_ssid));
		memcpy(&wifi_config.sta.ssid, registers.wifi_ssid, sizeof(registers.wifi_ssid));
		wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0';
		
		// Copy PASSWORD
		assert(sizeof(wifi_config.sta.password) == sizeof(registers.wifi_password));
		memcpy(&wifi_config.sta.password, registers.wifi_password, sizeof(registers.wifi_password));
		wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0';

		// Debug
		ESP_LOGI(TAG, "SSID: '%s', PASSWORD '%s'", wifi_config.sta.ssid, wifi_config.sta.password);
				
		// WiFi start internal state machine
		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
		ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
		ESP_ERROR_CHECK(esp_wifi_start());

		// Wait for error or another request
		xEventGroupWaitBits(wos_event_group, ESP32_REGISTER_ADDRESS_STATUS_WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
		
		// Error!		
		ESP_LOGE(TAG, "WiFi connection failed! Reset...");
		
		// WiFi stop internal state machine
		esp_wifi_stop();
	}

	// The event will not be processed after unregister
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	
	vTaskDelete(NULL);
}

/// ####################################################################################################################
/// SPI Slave
/// ####################################################################################################################
#define GPIO_MOSI         5
#define GPIO_MISO         4
#define GPIO_SCLK         6
#define GPIO_CS           7
#define RCV_HOST          SPI3_HOST
#define SPI_SLAVE_VERBOSE 0

// Main application
void spi_slave_task(void *pvParameters)
{
	esp_err_t ret;

	// Configuration for the SPI bus
	spi_bus_config_t buscfg = {
		.mosi_io_num = GPIO_MOSI,
		.miso_io_num = GPIO_MISO,
		.sclk_io_num = GPIO_SCLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
	};

	// Configuration for the SPI slave interface
	spi_slave_interface_config_t slvcfg = {
		.mode = 0,
		.spics_io_num = GPIO_CS,
		.queue_size = 3,
		.flags = 0,
	};

	// Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
	gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);


	// Initialise SPI slave interface
	ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
	assert(ret == ESP_OK);

	static WORD_ALIGNED_ATTR char sendbuf[ESP32_SPI_MESSAGE_MAX_SIZE] = "";
	static WORD_ALIGNED_ATTR char recvbuf[ESP32_SPI_MESSAGE_MAX_SIZE] = "";
	memset(recvbuf, 0, 33);
	spi_slave_transaction_t t;
	memset(&t, 0, sizeof(t));

	while(1) {
		// Clear receive buffer, set send buffer to something sane
		memset(sendbuf, 0xA5, sizeof(sendbuf));
		memset(recvbuf, 0xA5, sizeof(recvbuf));

		memcpy(sendbuf, "123456789", 9);

		// Set up a transaction of 128 bytes to send/receive
		t.length = ESP32_SPI_MESSAGE_MAX_SIZE * 8;
		t.tx_buffer = sendbuf;
		t.rx_buffer = recvbuf;
		
		ret = spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY);
		if (ret == ESP_OK) {	
			size_t received_bytes = (int)t.trans_len / 8;
			
			if(recvbuf[0] == 0x01) {
				// COMMAND_WRITE
				if(received_bytes > 3) {
					uint16_t address = (recvbuf[2] << 8U) | recvbuf[1];
					uint16_t count = received_bytes - 3;
					if(address != ESP32_REGISTER_ADDRESS_CONTROLSTATUS) {
						// Perform copy
						#define min(a,b) ((a) < (b) ? (a) : (b))
						uint16_t max_copy_bytes = sizeof(registers) - (address % sizeof(registers));
						memcpy((void*)((uint8_t*)&registers + address), &recvbuf[3], min(max_copy_bytes, count));

						// Debug
						#if SPI_SLAVE_VERBOSE
							printf("Write @ 0x%04X: ", address);
							for(uint32_t i = 3; i < received_bytes; ++i) {
								printf("%02X ", recvbuf[i]);
							}
							printf("\n");
						#endif // SPI_SLAVE_VERBOSE
					} else {
						xEventGroupSetBits(wos_event_group, recvbuf[3]);
						#if SPI_SLAVE_VERBOSE
							printf("Write @ 0x%04X (SET-CONTROL): %02X\n", address, recvbuf[3]);
						#endif // SPI_SLAVE_VERBOSE
					}
				}
			} else if(recvbuf[0] == 0x02) {
				// COMMAND_READ
				uint16_t address = (recvbuf[2] << 8U) | recvbuf[1];
				uint16_t count   = (recvbuf[4] << 8U) | recvbuf[3];

				// Clear receive buffer, set send buffer to something sane
				memset(sendbuf, 0xA5, sizeof(sendbuf));
				memset(recvbuf, 0xA5, sizeof(recvbuf));

				if(address != ESP32_REGISTER_ADDRESS_CONTROLSTATUS) {
					// Load memory to DMA buffer
					uint16_t max_copy_bytes = sizeof(registers) - (address % sizeof(registers));
					memcpy(sendbuf, (void*)((uint8_t*)&registers + address), min(max_copy_bytes, min(sizeof(sendbuf), count)));

					#if SPI_SLAVE_VERBOSE
						printf("Read  @ 0x%04X count %d: ", address, count);
						for(uint32_t i = 0; i < count; ++i) {
							printf("%02X ", sendbuf[i]);
						}
						printf("\n");
					#endif // SPI_SLAVE_VERBOSE
				} else {
					sendbuf[0] = _HI(xEventGroupGetBits(wos_event_group));
					#if SPI_SLAVE_VERBOSE
						printf("Read @ 0x%04X (GET-STATUS): %02X\n", address, sendbuf[0]);
					#endif // SPI_SLAVE_VERBOSE
				}

				// Set up a transaction
				t.length = ESP32_SPI_MESSAGE_MAX_SIZE * 8;
				t.tx_buffer = sendbuf;
				t.rx_buffer = recvbuf;
				spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY);
			} else {
				#if SPI_SLAVE_VERBOSE
					printf("SPI Error\n");
				#endif // SPI_SLAVE_VERBOSE
			}
		}
	}
}

/// ####################################################################################################################
/// Main
/// ####################################################################################################################

void app_main(void)
{	
	// Initialise NVS
	esp_err_t ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	
	wos_event_group = xEventGroupCreate();

	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	
	gpio_set_direction(GPIO_MOSI, GPIO_MODE_INPUT);
	gpio_set_direction(GPIO_MISO, GPIO_MODE_INPUT);
	gpio_set_direction(GPIO_SCLK, GPIO_MODE_INPUT);
	gpio_set_direction(GPIO_CS, GPIO_MODE_INPUT);

	xTaskCreate(spi_slave_task, "spi_slave_task", 4096, NULL, 5, NULL);	
	xTaskCreate(led_task, "led_task", 4096, NULL, 5, NULL);
	xTaskCreate(wifi_sta_task, "wifi_sta_task", 4096, NULL, 5, NULL);
	xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}

