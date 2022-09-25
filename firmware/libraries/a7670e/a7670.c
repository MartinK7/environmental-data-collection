
#include "a7670.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "usart.h"
#include "gpio.h"
#include "systick.h"

#include "FreeRTOS.h"
#include "task.h"

int8_t a7670_init(const char *apn)
{
	usart_init(UART4, 115200);
	//TODO
	return 0;
}

int8_t a7670_deinit(void)
{
	//TODO
	//usart_deinit(UART4);
	return 0;
}

int8_t a7670_tcp_transaction(const uint8_t *buffer_tx, uint16_t count_tx)
{
	//TODO
	return 0;
}

