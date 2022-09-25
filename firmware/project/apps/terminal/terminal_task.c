
#include "terminal_task.h"

#include <stdio.h>

#include "gpio.h"
#include "usart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#define TERMINAL_UART UART7
static StreamBufferHandle_t stream_from_uart = NULL;
static StreamBufferHandle_t stream_from_stdout = NULL;

static void terminal_isr(USART_TypeDef *usart, uint8_t data)
{
	BaseType_t xHigherPriorityTaskWoken;
	xStreamBufferSendFromISR(stream_from_uart, &data, 1, &xHigherPriorityTaskWoken);
}

static void terminal_task(void *parameters)
{
	for(;;) {
		char ch;
		if(1 == xStreamBufferReceive(stream_from_uart, &ch, 1, 0)) {
			static char lastch = 0;
			if(ch == '\n' && lastch != '\r') {
				usart_transmit(TERMINAL_UART, (const uint8_t*)"\r", 1);
			}
			usart_transmit(TERMINAL_UART, (const uint8_t*)&ch, 1);
			lastch = ch;
		} else if(1 == xStreamBufferReceive(stream_from_stdout, &ch, 1, 0)) {
			static char lastch = 0;
			if(ch == '\n' && lastch != '\r') {
				usart_transmit(TERMINAL_UART, (const uint8_t*)"\r", 1);
			}
			usart_transmit(TERMINAL_UART, (const uint8_t*)&ch, 1);
			lastch = ch;
		} else {
			taskYIELD();
		}
	}
}

void terminal_task_initialise(void)
{
	static uint8_t stream_from_uart_storage[64 + 1];
	static StaticStreamBuffer_t stream_from_uart_buffer;
	stream_from_uart = xStreamBufferCreateStatic(sizeof(stream_from_uart_storage), 1, stream_from_uart_storage, &stream_from_uart_buffer);

	static uint8_t stream_from_stdout_storage[2048 + 1];
	static StaticStreamBuffer_t stream_from_stdout_buffer;
	stream_from_stdout = xStreamBufferCreateStatic(sizeof(stream_from_stdout_storage), 1, stream_from_stdout_storage, &stream_from_stdout_buffer);

	usart_init(TERMINAL_UART, 115200);
	usart_attach_interrupt(TERMINAL_UART, terminal_isr);

	static StaticTask_t xTaskBuffer;
	static StackType_t xStack[2048];
	xTaskCreateStatic(terminal_task, "TERMINAL", sizeof(xStack) / sizeof(*xStack), NULL, 1, xStack, &xTaskBuffer);
}

void terminal_task_stdout_callback(const char *data, uint32_t size)
{
	if(stream_from_stdout) {
		xStreamBufferSend(stream_from_stdout, data, size, portMAX_DELAY);
	}
}
