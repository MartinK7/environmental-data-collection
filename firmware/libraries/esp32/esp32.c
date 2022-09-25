
#include "esp32.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "spi.h"
#include "gpio.h"
#include "systick.h"

#include "FreeRTOS.h"
#include "task.h"

#define ESP32_SPI                      SPI1
#define ESP32_DELAY_BEFORE_TRANSFER_MS 20

#define _LO(x) ((x) & 0x00FFU)
#define _HI(x) (((x) & 0xFF00U) >> 8U)

static uint8_t temp_tx[ESP32_SPI_MESSAGE_MAX_SIZE];

static int8_t esp32_register_write(uint16_t address, const uint8_t *buffer_tx, uint16_t count_tx)
{
	const uint16_t block = sizeof(temp_tx) - 8;
	uint16_t count_total = 0;
	while(count_total < count_tx) {
		uint16_t count_now = (count_tx - count_total) > block ? block : (count_tx - count_total);

		temp_tx[0] = 0x01; // COMMAND_WRITE
		temp_tx[1] = _LO(address + count_total);
		temp_tx[2] = _HI(address + count_total);
		memcpy(&temp_tx[3], buffer_tx + count_total, count_now);

		vTaskDelay(ESP32_DELAY_BEFORE_TRANSFER_MS / portTICK_PERIOD_MS);
		gpio_pin_write(GPIO_PIN_ESP32_CS, GPIO_PIN_STATE_LOW);
		spi_master_transaction(ESP32_SPI, temp_tx, count_now + 3, NULL, 0);
		gpio_pin_write(GPIO_PIN_ESP32_CS, GPIO_PIN_STATE_HIGH);

		count_total += count_now;
	}

	return 0;
}

static int8_t esp32_register_read(uint16_t address, uint8_t *buffer_rx, uint16_t count_rx)
{
	temp_tx[0] = 0x02; // COMMAND_READ
	temp_tx[1] = _LO(address);
	temp_tx[2] = _HI(address);
	temp_tx[3] = _LO(count_rx);
	temp_tx[4] = _HI(count_rx);

	// Read request
	vTaskDelay(ESP32_DELAY_BEFORE_TRANSFER_MS / portTICK_PERIOD_MS);
	gpio_pin_write(GPIO_PIN_ESP32_CS, GPIO_PIN_STATE_LOW);
	spi_master_transaction(ESP32_SPI, temp_tx, 5, NULL, 0);
	gpio_pin_write(GPIO_PIN_ESP32_CS, GPIO_PIN_STATE_HIGH);

	// Perform read
	vTaskDelay(ESP32_DELAY_BEFORE_TRANSFER_MS / portTICK_PERIOD_MS);
	gpio_pin_write(GPIO_PIN_ESP32_CS, GPIO_PIN_STATE_LOW);
	spi_master_transaction(ESP32_SPI, temp_tx, count_rx, buffer_rx, count_rx);
	gpio_pin_write(GPIO_PIN_ESP32_CS, GPIO_PIN_STATE_HIGH);

	return 0;
}

static int8_t esp32_register_control_setbits(uint8_t bits)
{
	return esp32_register_write(ESP32_REGISTER_ADDRESS_CONTROLSTATUS, &bits, 1);
}

static int8_t esp32_register_status_waitbits(uint8_t bits, uint8_t *bits_return, uint32_t timeout)
{
	uint32_t timeout_start = systick_get();
	do {
		if(systick_check_timeout(timeout_start, timeout)) {
			return -10;
		}
		int8_t r = esp32_register_read(ESP32_REGISTER_ADDRESS_CONTROLSTATUS, bits_return, 1);
		if(r) {
			return r;
		}
	} while(!(*bits_return & bits));
	return 0;
}

int8_t esp32_init(const char *ssid, const char *password)
{
	uint8_t bits;

	gpio_pin_write(GPIO_PIN_WIFI_EN, GPIO_PIN_STATE_LOW);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	spi_master_init(ESP32_SPI, 10000000U);
	gpio_pin_write(GPIO_PIN_WIFI_EN, GPIO_PIN_STATE_HIGH);
	vTaskDelay(4000 / portTICK_PERIOD_MS);

	while(1) {
		printf("Write WiFi settings...\r\n");
		esp32_register_write(ESP32_REGISTER_ADDRESS_WIFI_SSID, (const uint8_t*)ssid, strlen(ssid) + 1);
		esp32_register_write(ESP32_REGISTER_ADDRESS_WIFI_PASSWORD, (const uint8_t*)password, strlen(password) + 1);

		// Connect WiFi request
		printf("WiFi Connect request...\r\n");
		esp32_register_control_setbits(ESP32_REGISTER_ADDRESS_CONTROL_WIFI_CONNECT_REQUEST_BIT);

		int8_t r = esp32_register_status_waitbits(ESP32_REGISTER_ADDRESS_STATUS_WIFI_CONNECTED_BIT | ESP32_REGISTER_ADDRESS_STATUS_WIFI_FAIL_BIT, &bits, 30000);

		if(r) {
			return r;
		}

		if(bits & ESP32_REGISTER_ADDRESS_STATUS_WIFI_CONNECTED_BIT) {
			printf("WiFi Connect request success\r\n");
			break;
		}

		if(bits & ESP32_REGISTER_ADDRESS_STATUS_WIFI_FAIL_BIT) {
			printf("WiFi Connect request failed!\r\n");
			return -1;
		}
	}

	return 0;
}

int8_t esp32_deinit(void)
{
	spi_master_deinit(ESP32_SPI);
	gpio_pin_write(GPIO_PIN_WIFI_EN, GPIO_PIN_STATE_LOW);
	return 0;
}

int8_t esp32_tcp_open(const char *url, uint16_t port)
{
	uint8_t bits, tmp[2];
	int8_t r = 0;

	esp32_register_write(ESP32_REGISTER_ADDRESS_TCP_DOMAIN, (const uint8_t*)url, 15);
	tmp[0] = _LO(port);
	tmp[1] = _HI(port);
	esp32_register_write(ESP32_REGISTER_ADDRESS_TCP_PORT, tmp, 2);

	printf("TCP Connect request...\r\n");
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	esp32_register_control_setbits(ESP32_REGISTER_ADDRESS_CONTROL_TCP_CONNECT_REQUEST_BIT);

	printf("Wait until connects...\r\n");
	r = esp32_register_status_waitbits(
			ESP32_REGISTER_ADDRESS_STATUS_TCP_CONNECTED_BIT |
			ESP32_REGISTER_ADDRESS_STATUS_TCP_FAIL_BIT |
			ESP32_REGISTER_ADDRESS_STATUS_WIFI_FAIL_BIT,
			&bits, 30000);

	if(r) {
		return r;
	}

	if(bits & ESP32_REGISTER_ADDRESS_STATUS_WIFI_FAIL_BIT) {
		return -1;
	}

	if(bits & ESP32_REGISTER_ADDRESS_STATUS_TCP_FAIL_BIT) {
		return -2;
	}

	return 0;
}

int8_t esp32_tcp_write(const uint8_t *buffer_tx, uint32_t count_tx)
{
	uint8_t bits, tmp[2];
	int8_t r = 0;

	// Check already connected
	if(esp32_register_read(ESP32_REGISTER_ADDRESS_CONTROLSTATUS, &bits, 1)) {
		return -1;
	}
	if(!(bits & ESP32_REGISTER_ADDRESS_STATUS_TCP_CONNECTED_BIT)) {
		return 0;
	}

	const uint32_t block = ESP32_REGISTER_MAX_SIZE - 16;//TODO ESP32_REGISTER_MAX_SIZE;
	uint32_t count_total = 0;
	while(count_total < count_tx) {
		uint32_t count_now = (count_tx - count_total) > block ? block : (count_tx - count_total);

		esp32_register_write(ESP32_REGISTER_ADDRESS_BUFFER_TX, buffer_tx + count_total, count_now);
		tmp[0] = _LO(count_now);
		tmp[1] = _HI(count_now);
		esp32_register_write(ESP32_REGISTER_ADDRESS_BUFFER_TX_LEN, tmp, 2);

		esp32_register_control_setbits(ESP32_REGISTER_ADDRESS_CONTROL_TCP_SEND_REQUEST_BIT);

		r = esp32_register_status_waitbits(ESP32_REGISTER_ADDRESS_STATUS_TCP_SENT_BIT | ESP32_REGISTER_ADDRESS_STATUS_TCP_FAIL_BIT, &bits, 30000);

		if(r || (bits & ESP32_REGISTER_ADDRESS_STATUS_TCP_FAIL_BIT)) {
			// Try to close socket
			esp32_register_control_setbits(ESP32_REGISTER_ADDRESS_CONTROL_TCP_DISCONNECT_REQUEST_BIT);
			esp32_register_status_waitbits(ESP32_REGISTER_ADDRESS_STATUS_TCP_DISCONNECTED_BIT, &bits, 30000);
			// Already failed
			return -4;
		}

		count_total += count_now;
	}

	return 0;
}

int8_t esp32_tcp_close(void)
{
	uint8_t bits;
	int8_t r = 0;

	// Check already connected
	if(esp32_register_read(ESP32_REGISTER_ADDRESS_CONTROLSTATUS, &bits, 1)) {
		return -1;
	}
	if(!(bits & ESP32_REGISTER_ADDRESS_STATUS_TCP_CONNECTED_BIT)) {
		return 0;
	}

	esp32_register_control_setbits(ESP32_REGISTER_ADDRESS_CONTROL_TCP_DISCONNECT_REQUEST_BIT);

	r = esp32_register_status_waitbits(ESP32_REGISTER_ADDRESS_STATUS_TCP_DISCONNECTED_BIT, &bits, 30000);

	if(r) {
		return r;
	}

	return 0;
}
