
#ifndef DRIVERS_USART_H_
#define DRIVERS_USART_H_

#include "stm32h7xx.h"

int8_t usart_init(USART_TypeDef *usart, uint32_t baudrate);
uint32_t usart_transmit(USART_TypeDef *usart, const uint8_t *data, uint32_t size);
uint32_t usart_receive(USART_TypeDef *usart, uint8_t *data, uint32_t size);
void usart_attach_interrupt(USART_TypeDef *usart, void(*callback)(USART_TypeDef *usart, uint8_t data));

#endif /* DRIVERS_USART_H_ */
