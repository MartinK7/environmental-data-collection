
#include "usart.h"
#include "systick.h"
#include "rcc.h"
#include "driver_os.h"

static struct {
	USART_TypeDef *usart_target;
	void(*callback)(USART_TypeDef *usart, uint8_t data);
} callbacks[10] = {0};

static int8_t usart_init_reset(USART_TypeDef *usart, uint32_t *clock)
{
	__disable_irq();
	switch((uint32_t)usart) {
		case (uint32_t)UART5:
				RCC->APB1LENR  |=   RCC_APB1LENR_UART5EN;
				RCC->APB1LRSTR |=   RCC_APB1LRSTR_UART5RST;
				RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_UART5RST);
				*clock = rcc_apb1_get_clock();
				break;
		case (uint32_t)UART7:
				RCC->APB1LENR  |=   RCC_APB1LENR_UART7EN;
				RCC->APB1LRSTR |=   RCC_APB1LRSTR_UART7RST;
				RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_UART7RST);
				*clock = rcc_apb1_get_clock();
				break;
		case (uint32_t)UART4:
				RCC->APB1LENR  |=   RCC_APB1LENR_UART4EN;
				RCC->APB1LRSTR |=   RCC_APB1LRSTR_UART4RST;
				RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_UART4RST);
				*clock = rcc_apb1_get_clock();
				break;
		case (uint32_t)USART1:
		case (uint32_t)USART2:
		case (uint32_t)USART3:
		case (uint32_t)USART6:
		case (uint32_t)UART8:
		case (uint32_t)UART9:
		case (uint32_t)USART10:
		default:
			__enable_irq();
			return -1;
	}
	__enable_irq();
	return 0;
}

static void usart_isr(USART_TypeDef *usart)
{
	uint32_t flags = usart->ISR;
	if(flags & USART_ISR_RXNE_RXFNE) {
		uint8_t data = *(__IO uint8_t*)&usart->RDR;
		for(uint32_t i = 0; i < sizeof(callbacks) / sizeof(*callbacks); ++i) {
			if(!callbacks[i].usart_target || !callbacks[i].callback) {
				// End of callback list
				break;
			}
			if(callbacks[i].usart_target == usart) {
				// Callback found!
				callbacks[i].callback(usart, data);
				break;
			}
		}
	}
	usart->ICR = flags;
}

void UART5_IRQHandler(void)
{
	usart_isr(UART5);
}

void UART7_IRQHandler(void)
{
	usart_isr(UART7);
}

void UART4_IRQHandler(void)
{
	usart_isr(UART4);
}

int8_t usart_init(USART_TypeDef *usart, uint32_t baudrate)
{
	uint32_t usart_pclock;

	// Reset
	if(usart_init_reset(usart, &usart_pclock)) {
		// Unknown USART peripheral
		return -1;
	}

	usart->CR1 &= ~(USART_CR1_UE | USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE | USART_CR1_OVER8);
	usart->CR1 |=   USART_CR1_TE | USART_CR1_RE;
	usart->CR2 &= ~(USART_CR2_STOP | USART_CR2_LINEN | USART_CR2_CLKEN);
	usart->CR3 &= ~(USART_CR3_RTSE | USART_CR3_CTSE | USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN);

	/*
	usart->CR3 |= USART_CR3_CTSE;

	usart->CR3 |= USART_CR3_RTSE;

	usart->CR3 |= USART_CR3_CTSE | USART_CR3_RTSE;

	usart->CR3 |= USART_CR3_DEM;
	usart->CR1 |= USART_CR1_DEAT | USART_CR1_DEDT;
	*/

	usart->BRR =  (usart_pclock + baudrate / 2U) / baudrate;
	usart->ICR =  0xFFFFFFFFU;
	usart->CR1 |= USART_CR1_UE;

	return 0;
}

uint32_t usart_transmit(USART_TypeDef *usart, const uint8_t *data, uint32_t size)
{
	uint32_t n = 0;
	while(n < size) {
		uint32_t timeout = systick_get();
		while(!(usart->ISR & USART_ISR_TXE_TXFNF)) {
			if(systick_check_timeout(timeout, 100)) {
				return n;
			}
			driver_os_yield();
		}
		n++;
		*(__IO uint8_t*)&usart->TDR = *(data++);
	}
	return n;
}

uint32_t usart_receive(USART_TypeDef *usart, uint8_t *data, uint32_t size)
{
	uint32_t n = 0;

	while(n < size) {
		uint32_t timeout = systick_get();
		while(!(usart->ISR & USART_ISR_RXNE_RXFNE)) {
			if(systick_check_timeout(timeout, 100)) {
				return n;
			}
			driver_os_yield();
		}
		n++;
		*(data++) = *(__IO uint8_t*)&usart->RDR;
	}
	return n;
}


void usart_attach_interrupt(USART_TypeDef *usart, void(*callback)(USART_TypeDef *usart, uint8_t data))
{
	// Lock this interface, next registers manipulation must be atomic
	__disable_irq();

	// Register callback
	for(uint32_t i = 0; i < sizeof(callbacks) / sizeof(*callbacks); ++i) {
		if(!callbacks[i].callback) {
			callbacks[i].usart_target = usart;
			callbacks[i].callback = callback;
			break;
		}
	}

	// Un-mask interrupt
//	while(!(usart->ISR & USART_ISR_TXFE) || (usart->ISR & USART_ISR_BUSY)) {}
//	usart->CR1 &= ~(USART_CR1_UE);
	usart->CR1 |=   USART_CR1_RXNEIE_RXFNEIE;
//	usart->CR1 |=   USART_CR1_UE;

	// Enable interrupt
	switch((uint32_t)usart) {
		case (uint32_t)UART5:
				NVIC_SetPriority(UART5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
				NVIC_EnableIRQ(UART5_IRQn);
				break;
		case (uint32_t)UART7:
				NVIC_SetPriority(UART7_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
				NVIC_EnableIRQ(UART7_IRQn);
				break;
		case (uint32_t)UART4:
				NVIC_SetPriority(UART7_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
				NVIC_EnableIRQ(UART7_IRQn);
				break;
		case (uint32_t)USART1:
		case (uint32_t)USART2:
		case (uint32_t)USART3:
		case (uint32_t)USART6:
		case (uint32_t)UART8:
		case (uint32_t)UART9:
		case (uint32_t)USART10:
		default:
			// NOP
			break;
	}

	// Unlock
	__enable_irq();
}

