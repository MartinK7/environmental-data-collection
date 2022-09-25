
#include "pms5003_task.h"

#include "gpio.h"
#include "usart.h"
#include "systick.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "semphr.h"

#define PMS5003_UART UART5

static StreamBufferHandle_t stream_from_pms5003_uart = NULL;
static SemaphoreHandle_t xSemaphore = NULL;

static struct pms5003_data data_public = {0};
static uint32_t data_public_timestamp = 0;

static void pms5003_isr(USART_TypeDef *usart, uint8_t data)
{
	BaseType_t xHigherPriorityTaskWoken;
	xStreamBufferSendFromISR(stream_from_pms5003_uart, &data, 1, &xHigherPriorityTaskWoken);
}

static void pms5003_task(void *parameters)
{
	for(;;) {
		char ch;
		xStreamBufferReceive(stream_from_pms5003_uart, &ch, 1, portMAX_DELAY);
		if(ch != 0x42) {
			continue;
		}
		xStreamBufferReceive(stream_from_pms5003_uart, &ch, 1, portMAX_DELAY);
		if(ch != 0x4d) {
			continue;
		}

		uint16_t data[15];
		for(size_t count = 0; count < sizeof(data);) {
			count += xStreamBufferReceive(stream_from_pms5003_uart, (uint8_t*)data + count, sizeof(data) - count, portMAX_DELAY);
		}
		for(size_t i = 0; i < sizeof(data) / sizeof(*data); ++i) {
			data[i] = __REVSH(data[i]);
		}

		// Check frame length.
		if(data[0] != 2 * 13 + 2) {
			continue;
		}

		// Check sum.
		size_t check_sum = 0x42 + 0x4d;
		for(size_t i = 0; i < sizeof(data); ++i) {
			check_sum += ((uint8_t*)data)[i];
		}
		if(check_sum != data[14]) {
			//continue;
		}

		// Atomic update
		xSemaphoreTake(xSemaphore, portMAX_DELAY);
		for(size_t i = 1; i <= 12; ++i) {
			data_public.raw[i - 1] = (float)data[i];
		}
		data_public_timestamp = systick_get();
		xSemaphoreGive(xSemaphore);
	}
}

int8_t pms5003_get_data(struct pms5003_data *data)
{
	// Atomic copy
	xSemaphoreTake(xSemaphore, portMAX_DELAY);
	if(data_public_timestamp == 0 || systick_check_timeout(data_public_timestamp, 60 * 1000)) {
		xSemaphoreGive(xSemaphore);
		return -1;
	}
	*data = data_public;
	xSemaphoreGive(xSemaphore);
	return 0;
}

int8_t pms5003_power_on(void)
{
	xSemaphoreTake(xSemaphore, portMAX_DELAY);
	data_public_timestamp = 0;
	xSemaphoreGive(xSemaphore);

	gpio_pin_write(GPIO_PIN_CON_UART5_5V_EN,   GPIO_PIN_STATE_HIGH);
	vTaskDelay(500 / portTICK_PERIOD_MS);
	gpio_pin_write(GPIO_PIN_CON_PMS_SET,       GPIO_PIN_STATE_HIGH);
	gpio_pin_write(GPIO_PIN_CON_PMS_RESET,     GPIO_PIN_STATE_HIGH);
	vTaskDelay(500 / portTICK_PERIOD_MS);

	return 0;
}

int8_t pms5003_power_off(void)
{
	xSemaphoreTake(xSemaphore, portMAX_DELAY);
	data_public_timestamp = 0;
	xSemaphoreGive(xSemaphore);

	gpio_pin_write(GPIO_PIN_CON_PMS_SET,       GPIO_PIN_STATE_LOW);
	gpio_pin_write(GPIO_PIN_CON_PMS_RESET,     GPIO_PIN_STATE_LOW);
	vTaskDelay(500 / portTICK_PERIOD_MS);
	gpio_pin_write(GPIO_PIN_CON_UART5_5V_EN,   GPIO_PIN_STATE_LOW);
	vTaskDelay(500 / portTICK_PERIOD_MS);

	return 0;
}

void pms5003_task_initialise(void)
{
	static uint8_t stream_from_pms5003_uart_storage[64 + 1];
	static StaticStreamBuffer_t stream_from_pms5003_uart_buffer;
	stream_from_pms5003_uart = xStreamBufferCreateStatic(sizeof(stream_from_pms5003_uart_storage), 1, stream_from_pms5003_uart_storage, &stream_from_pms5003_uart_buffer);

	static StaticSemaphore_t xMutexBuffer;
	xSemaphore = xSemaphoreCreateMutexStatic(&xMutexBuffer);
	configASSERT(xSemaphore);

	usart_init(PMS5003_UART, 9600);
	usart_attach_interrupt(PMS5003_UART, pms5003_isr);

	static StaticTask_t xTaskBuffer;
	static StackType_t xStack[512];
	xTaskCreateStatic(pms5003_task, "PMS5003", sizeof(xStack) / sizeof(*xStack), NULL, 1, xStack, &xTaskBuffer);
}
