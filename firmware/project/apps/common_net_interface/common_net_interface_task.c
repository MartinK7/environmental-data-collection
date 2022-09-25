
#include "common_net_interface_task.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "semphr.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "../../esp32/esp32.h"
#include "../../a7670e/a7670.h"

#include "systick.h"
#include "stmflash_storage.h"

#if 0
string:hex_tdelta:hex_chksum_unsigned_char:
EDCASCII:00000000:00000000:sensor1=45.25,sensor2=7,sensor=88 ...

EDCBINARY:
#endif

static struct stmflash_storage_databock settings;

static SemaphoreHandle_t xSemaphore = NULL;
static MessageBufferHandle_t xMessageBuffer = NULL;

#if !defined(NO_ETH)
static int8_t ethernet_open(const char *url, uint16_t port);
static int8_t ethernet_write(const uint8_t *buffer_tx, uint32_t count_tx);
static int8_t ethernet_close(void);
#endif

static struct cni {
	const char *name;
	const uint8_t *enable_ptr;
	// TCP
	int8_t (*open)(const char *url, uint16_t port);
	int8_t(*write)(const uint8_t *buffer, uint32_t count);
	int8_t(*close)(void);
} inferfaces[] = {
	{
		.name = "WiFi-ESP32",
		.enable_ptr = &settings.wifi.enable,
		.open = esp32_tcp_open,
		.write = esp32_tcp_write,
		.close = esp32_tcp_close
	}
#if !defined(NO_ETH)
	,{
		.name = "Ethernet-LAN8742",
		.enable_ptr = &settings.ethernet.enable,
		.open = ethernet_open,
		.write = ethernet_write,
		.close = ethernet_close
	}
#endif
};

#if !defined(NO_ETH)

static int ethernet_socket = 0;

static int8_t ethernet_open(const char *url, uint16_t port)
{
	// Resolve IP first
	ip_addr_t dns_addr;
	for(uint32_t i = 0;; ++i) {
		if(i >= 10) {
			return -1;
		}
		if(!dns_gethostbyname(url, &dns_addr, NULL, NULL)) {
			// Found
			break;
		}
		// Not found
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	// Create the socket
	ethernet_socket = lwip_socket(AF_INET, SOCK_STREAM, 0);

	if(ethernet_socket < 0) {
		// Socket create failed
		return -1;
	}

	// Setup address
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_len = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_port = PP_HTONS(port);
	addr.sin_addr.s_addr = dns_addr.addr;

	// Connect
	int ret = lwip_connect(ethernet_socket, (struct sockaddr*)&addr, sizeof(addr));

	if(ret != 0) {
		// Connection failed
		lwip_close(ethernet_socket);
		return -1;
	}

	return 0;
}

static int8_t ethernet_write(const uint8_t *buffer_tx, uint32_t count_tx)
{
	// Write something
	int ret = lwip_write(ethernet_socket, buffer_tx, count_tx);
	if(ret != count_tx) {
		// Upload error
		lwip_close(ethernet_socket);
		ethernet_socket = 0;
		return -1;
	}

	return 0;
}

static int8_t ethernet_close(void)
{
	if(!ethernet_socket) {
		return 0;
	}

	// Close
	int ret = lwip_close(ethernet_socket);

	if(ret) {
		return -1;
	}

	return 0;
}
#endif // !NO_ETH

static void cni_task(void *parameter)
{
	if(stmflash_storage_read(&settings)) {
		//TODO error?
		return;
	}

#if 0
	//TODO move this into terminal/shell task
	//snprintf(settings.wifi.ssid, sizeof(settings.wifi.ssid), "%s", "");
	//snprintf(settings.wifi.password, sizeof(settings.wifi.password), "%s", "");
	settings.server.port = 3002;
	snprintf(settings.server.url, sizeof(settings.server.url), "%s", "pony.zapto.org");
	settings.ethernet.enable = 1;
	settings.wifi.enable = 0;
	settings.gases.resistor_nox_up = 10000;
	settings.gases.resistor_nox_down = 6770;
	settings.gases.resistor_co_up = 0;
	settings.gases.resistor_co_down = 47060;
	settings.gases.resistor_o3_up = 744000;
	settings.gases.resistor_o3_down = 220300 + 29950;
	settings.gases.resistor_5Vref_up = 3911;
	settings.gases.resistor_5Vref_down = 2190;
	stmflash_storage_program(&settings);
#endif

	if(settings.wifi.enable) {
		esp32_init(settings.wifi.ssid, settings.wifi.password);
	}

	if(settings.gsmlte.enable) {
		a7670_init("");
	}

	while(1)
	{
		// Wait for data
		static uint8_t upload_buffer[CNI_UPLOAD_DATA_MAX_SIZE];
		static uint16_t upload_buffer_size;
		upload_buffer_size = xMessageBufferReceive(xMessageBuffer, upload_buffer, sizeof(upload_buffer), pdMS_TO_TICKS(100));
		if(upload_buffer_size == 0) {
			// There is nothing to upload
			continue;
		}

		cni_upload_timestamped_data_blocking(upload_buffer, upload_buffer_size, 60000);
	}
}

int8_t cni_upload_timestamped_data_blocking(const uint8_t *timestamped_data, uint32_t timestamped_data_size, uint32_t timeout)
{
	if(timestamped_data_size <= sizeof(uint32_t)) {
		return -1;
	}

	uint32_t timestamp = *((const uint32_t*)timestamped_data);
	timestamped_data += sizeof(uint32_t);
	timestamped_data_size -= sizeof(uint32_t);

	if(xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(timeout)) == pdFALSE) {
		return -1;
	}

	for(struct cni *iface = &inferfaces[0]; iface < &inferfaces[sizeof(inferfaces) / sizeof(*inferfaces)]; ++iface) {
		if(!(*iface->enable_ptr)) {
			continue;
		}

		if(iface->open(settings.server.url, settings.server.port)) {
			continue;
		}

		// Generate HTTP POST header
		static char http_post_head[128];
		snprintf(http_post_head, sizeof(http_post_head),
			"POST %s HTTP/1.1\n"
			"Content-Length: %"PRIu32"\n"
			"X-Message-Age: %"PRIu32"\n"
			"X-Originating-Device: %s\n"
			"\n",

			"/upload",
			timestamped_data_size,
			(systick_get() - timestamp) / (uint32_t)1000,
			iface->name
		);

		if(iface->write((const uint8_t*)http_post_head, strlen(http_post_head))) {
			iface->close();
			continue;
		}

		if(iface->write(timestamped_data, timestamped_data_size)) {
			iface->close();
			continue;
		}

		iface->close();
	}

	xSemaphoreGive(xSemaphore);

	vTaskDelay(pdMS_TO_TICKS(50));

	return 0;
}

int8_t cni_upload_timestamped_data(const uint8_t *timestamped_data, uint32_t timestamped_data_size)
{
	if(timestamped_data_size <= sizeof(uint32_t)) {
		return -1;
	}

	if(timestamped_data_size >= CNI_UPLOAD_DATA_MAX_SIZE) {
		return -1;
	}

	if(timestamped_data_size != xMessageBufferSend(xMessageBuffer, timestamped_data, timestamped_data_size, 0)) {
		return -2;
	}

	return 0;
}

void cni_task_initialise(void)
{
	static StaticSemaphore_t xMutexBuffer;
	xSemaphore = xSemaphoreCreateMutexStatic(&xMutexBuffer);
	configASSERT(xSemaphore);

	#define STORAGE_SIZE_BYTES ((CNI_UPLOAD_DATA_MAX_SIZE + sizeof(size_t)) * 4)
	static uint8_t ucMessageBufferStorage[STORAGE_SIZE_BYTES];
	static StaticMessageBuffer_t xMessageBufferStruct;
	xMessageBuffer = xMessageBufferCreateStatic(sizeof(ucMessageBufferStorage), ucMessageBufferStorage, &xMessageBufferStruct);

	#define STACK_SIZE 2048
	static StaticTask_t xTaskBuffer;
	static StackType_t xStack[STACK_SIZE];
	xTaskCreateStatic(cni_task, "CNI", STACK_SIZE, NULL, 1, xStack, &xTaskBuffer);
}
