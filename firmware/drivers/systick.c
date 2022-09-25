
#include "stm32h7xx.h"
#include "driver_os.h"

static volatile uint32_t systick_counter_milliseconds = 0;
static volatile void(*systick_irq_callbacks[4])(void) = {0};

void SysTick_Handler(void)
{
	// Clear overflow flag
	SysTick->CTRL;

	++systick_counter_milliseconds;

	for(uint32_t i = 0; i < sizeof(systick_irq_callbacks) / sizeof(*systick_irq_callbacks); ++i) {
		if(systick_irq_callbacks[i] == (void*)(0)) {
			return;
		}
		systick_irq_callbacks[i]();
	}
}

void systick_init(void)
{
	SysTick_Config(SystemCoreClock / 1000);

	NVIC_SetPriorityGrouping(0x03U);
	NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
	NVIC_EnableIRQ(SysTick_IRQn);
}

uint32_t systick_get(void)
{
	return systick_counter_milliseconds;
}

int8_t systick_check_timeout(uint32_t start_time_milliseconds, uint32_t timeout_milliseconds)
{
	return (systick_counter_milliseconds - start_time_milliseconds) > timeout_milliseconds ? 1 : 0;
}

void systick_delay_milliseconds(uint32_t duration_milliseconds)
{
	uint32_t start = systick_get();
	while(!systick_check_timeout(start, duration_milliseconds)) {
		driver_os_yield();
	}
}

void systick_irq_add_callback(void(*cb)(void))
{
	for(uint32_t i = 0; i < sizeof(systick_irq_callbacks) / sizeof(*systick_irq_callbacks); ++i) {
		if(systick_irq_callbacks[i] == (void*)(0)) {
			systick_irq_callbacks[i] = cb;
			return;
		}
	}
}
