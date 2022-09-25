
#include "tim.h"

#include "rcc.h"

static int8_t tim_init_reset(TIM_TypeDef *tim, uint32_t *clock)
{
	__disable_irq();
	switch((uint32_t)tim) {
		case (uint32_t)TIM1:
			RCC->APB2ENR   |=   RCC_APB2ENR_TIM1EN;
			RCC->APB2RSTR  |=   RCC_APB2RSTR_TIM1RST;
			RCC->APB2RSTR  &= ~(RCC_APB2RSTR_TIM1RST);
			*clock = rcc_apb2_timx_get_clock();
			break;
		case (uint32_t)TIM3:
			RCC->APB1LENR  |=   RCC_APB1LENR_TIM3EN;
			RCC->APB1LRSTR |=   RCC_APB1LRSTR_TIM3RST;
			RCC->APB1LRSTR &= ~(RCC_APB1LRSTR_TIM3RST);
			*clock = rcc_apb1_timx_get_clock();
			break;
		default:
			__enable_irq();
			return -1;
	}
	__enable_irq();
	return 0;
}

int8_t tim_clock_generator_init(TIM_TypeDef *tim, uint8_t channel, uint32_t frequency)
{
	uint32_t tim_pclock;

	// Reset
	if(tim_init_reset(tim, &tim_pclock)) {
		// Unknown TIM peripheral
		return -1;
	}

	uint32_t psc = (tim_pclock * 2) / frequency;
	if(psc) {
		--psc;
	} else {
		// tim_pclock too slow
		return -1;
	}
	if(psc > 0xFFFFU) {
		// tim_pclock too fast
		return -2;
	}

	tim->ARR = 0x1;
	tim->PSC = psc;
	tim->EGR = TIM_EGR_UG;

	switch(channel) {
		case 1:
			tim->CCMR1 |=  (TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1);
			tim->CCER  |=  (TIM_CCER_CC1E);
			tim->CCR1   =  0x1;
			break;
		case 2:
			tim->CCMR1 |=  (TIM_CCMR1_OC2PE | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1);
			tim->CCER  |=  (TIM_CCER_CC2E);
			tim->CCR2   =  0x1;
			break;
		case 3:
			tim->CCMR2 |=  (TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1);
			tim->CCER  |=  (TIM_CCER_CC3E);
			tim->CCR3   =  0x1;
			break;
		case 4:
			tim->CCMR2 |=  (TIM_CCMR2_OC4PE | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1);
			tim->CCER  |=  (TIM_CCER_CC4E);
			tim->CCR4   =  0x1;
			break;
	}

	tim->CR1 |= (TIM_CR1_CEN);
	return 0;
}

int8_t tim_counter_init(TIM_TypeDef *tim, uint32_t ticks_per_second, uint16_t cnt_max, uint8_t reload_enable)
{
	uint32_t tim_pclock;

	// Reset
	if(tim_init_reset(tim, &tim_pclock)) {
		// Unknown TIM peripheral
		return -1;
	}

	uint32_t psc = tim_pclock / ticks_per_second;
	if(psc) {
		--psc;
	} else {
		// tim_pclock too slow
		return -1;
	}
	if(psc > 0xFFFFU) {
		// tim_pclock too fast
		return -2;
	}

	if(reload_enable) {
		tim->CR1 |= TIM_CR1_OPM;
	}

	tim->ARR = cnt_max;
	tim->PSC = psc;
	tim->EGR = TIM_EGR_UG;

	tim->CR1 |= TIM_CR1_CEN;
	return 0;
}

uint16_t tim_counter_get(TIM_TypeDef *tim)
{
	return tim->CNT;
}

int8_t tim_counter_restart(TIM_TypeDef *tim)
{
	tim->CR1 &= ~(TIM_CR1_CEN);
	tim->EGR  =   TIM_EGR_UG;
	tim->CR1 |=  (TIM_CR1_CEN);
	return 0;
}
