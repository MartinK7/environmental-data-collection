
#include "adc.h"
#include "rcc.h"
#include "systick.h"

static void(*adc1_callback)(uint32_t adc1_value);
static void(*adc2_callback)(uint32_t adc2_value);

static int8_t adc_init_reset(ADC_TypeDef *adc, uint32_t *clock)
{
	__disable_irq();
	switch((uint32_t)adc) {
		case (uint32_t)ADC1:
		case (uint32_t)ADC2:
			RCC->AHB1ENR   |=   RCC_AHB1ENR_ADC12EN;
			RCC->AHB1RSTR  |=   RCC_AHB1RSTR_ADC12RST;
			RCC->AHB1RSTR  &= ~(RCC_AHB1RSTR_ADC12RST);
			*clock = rcc_hsi_get_clock();
		case (uint32_t)ADC3:
			RCC->AHB4ENR   |=   RCC_AHB4ENR_ADC3EN;
			RCC->APB2RSTR  |=   RCC_APB2RSTR_SPI1RST;
			RCC->APB2RSTR  &= ~(RCC_APB2RSTR_SPI1RST);
			*clock = rcc_hsi_get_clock();
			break;
		default:
			__enable_irq();
			return -1;
	}
	__enable_irq();
	return 0;
}

void ADC1_2_IRQHandler(void)
{
	uint32_t adc1 = ADC1->DR;
	uint32_t adc2 = ADC2->DR;
	ADC1->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;
	ADC2->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;
	if(adc1_callback) {
		adc1_callback(adc1);
	}
	if(adc2_callback) {
		adc2_callback(adc2);
	}
}

int8_t adc1_and_adc2_init(void)
{
	uint32_t adc_pclock;

	// Reset
	if(adc_init_reset(ADC1, &adc_pclock)) {
		// Unknown ADC peripheral
		return -1;
	}

	SYSCFG->PMCR |= SYSCFG_PMCR_PC2SO;

	// ADC1
	ADC1->CR &= ~(ADC_CR_DEEPPWD | ADC_CR_ADCAL | ADC_CR_JADSTP | ADC_CR_ADSTP | ADC_CR_JADSTART | ADC_CR_ADSTART | ADC_CR_ADDIS | ADC_CR_ADEN);
	ADC1->CR |=  (ADC_CR_ADVREGEN);
	systick_delay_milliseconds(1);

	// ADC12 Clock
	ADC12_COMMON->CCR &= ~(ADC_CCR_CKMODE | ADC_CCR_PRESC);
	ADC12_COMMON->CCR |=  (ADC_CCR_PRESC_0);

	// ADC1
	ADC1->CFGR &= ~(ADC_CFGR_RES | ADC_CFGR_CONT | ADC_CFGR_DISCEN | ADC_CFGR_EXTEN | ADC_CFGR_OVRMOD | ADC_CFGR_DISCNUM | ADC_CFGR_EXTSEL | ADC_CFGR_DMNGT | ADC_CFGR_AUTDLY);
	ADC1->CFGR |=  (ADC_CFGR_EXTSEL_2 | ADC_CFGR_EXTEN_0);
	ADC1->CFGR &= ~(ADC_CFGR2_ROVSE | ADC_CFGR2_LSHIFT);
	if((DBGMCU->IDCODE) >> 16 > 0x1003U)
	{
		ADC1->CR &= ~(ADC_CR_BOOST);
		ADC1->CR |=  (ADC_CR_BOOST_0);
	} else {
		ADC1->CR &= ~(ADC_CR_BOOST_0);
	}
//	ADC1->SQR1  &= ~(ADC_SQR1_L);
	ADC1->SMPR1 &= ~(ADC_SMPR1_SMP1);
	ADC1->SMPR1 |=  (ADC_SMPR1_SMP1_1 | ADC_SMPR1_SMP1_0);
	ADC1->CR &= ~(ADC_CR_ADCALDIF);
	ADC1->CR |= ADC_CR_ADCAL;
	while(ADC1->CR & ADC_CR_ADCAL) {};

	ADC1->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;

	// ADC2
	ADC2->CR &= ~(ADC_CR_DEEPPWD | ADC_CR_ADCAL | ADC_CR_JADSTP | ADC_CR_ADSTP | ADC_CR_JADSTART | ADC_CR_ADSTART | ADC_CR_ADDIS | ADC_CR_ADEN);
	ADC2->CR |=  (ADC_CR_ADVREGEN);
	systick_delay_milliseconds(1);

	ADC2->CFGR &= ~(ADC_CFGR_RES | ADC_CFGR_CONT | ADC_CFGR_DISCEN | ADC_CFGR_EXTEN | ADC_CFGR_OVRMOD | ADC_CFGR_DISCNUM | ADC_CFGR_EXTSEL | ADC_CFGR_DMNGT | ADC_CFGR_AUTDLY);
	ADC2->CFGR |=  (ADC_CFGR_EXTSEL_2 | ADC_CFGR_EXTEN_0);
	ADC2->CFGR &= ~(ADC_CFGR2_ROVSE | ADC_CFGR2_LSHIFT);
	if((DBGMCU->IDCODE) >> 16 > 0x1003U) // Rev Y
	{
		ADC2->CR &= ~(ADC_CR_BOOST);
		ADC2->CR |=  (ADC_CR_BOOST_0);
	} else {
		ADC2->CR &= ~(ADC_CR_BOOST_0);
	}
//	ADC2->SQR1  &= ~(ADC_SQR1_L);
	ADC2->SMPR1 &= ~(ADC_SMPR1_SMP1);
	ADC2->SMPR1 |=  (ADC_SMPR1_SMP1_1 | ADC_SMPR1_SMP1_0);
	ADC2->CR &= ~(ADC_CR_ADCALDIF);
	ADC2->CR |= ADC_CR_ADCAL;
	while(ADC2->CR & ADC_CR_ADCAL) {};

	ADC2->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;

	return 0;
}

int8_t adc_attach_callback(ADC_TypeDef *adc, void(*adc_callback)(uint32_t adc_value))
{
	if(adc == ADC1) {
		adc1_callback = adc_callback;
	} else if(adc == ADC2) {
		adc2_callback = adc_callback;
	} else {
		return -1;
	}
	return 0;
}

int8_t adc_start(ADC_TypeDef *adc)
{
	adc->IER |= ADC_IER_EOCIE;

	if(adc == ADC1 || adc == ADC2) {
		NVIC_SetPriority(ADC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 0));
		NVIC_EnableIRQ(ADC_IRQn);
	} else if(adc == ADC1) {
		NVIC_SetPriority(ADC3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 0));
		NVIC_EnableIRQ(ADC3_IRQn);
	} else {
		return -1;
	}

	adc->CR |= ADC_CR_ADEN;
	while(!(adc->ISR & ADC_ISR_ADRDY)) // TODO: Timeout!
		adc->CR |= ADC_CR_ADEN;
	return 0;
}

int8_t adc_stop(ADC_TypeDef *adc)
{
	adc->IER &= ~ADC_IER_EOCIE;
	adc->CR &= ~ADC_CR_ADEN;
	while((adc->ISR & ADC_ISR_ADRDY)) {}
	return 0;
}

int8_t adc_configure_channel(ADC_TypeDef *adc, uint32_t position, uint32_t channel, uint32_t total_positions_count)
{
	switch(position) {
		case 0:  adc->SQR1 &= ~ADC_SQR1_SQ1_Msk;  adc->SQR1 |= channel << ADC_SQR1_SQ1_Pos;  break;
		case 1:  adc->SQR1 &= ~ADC_SQR1_SQ2_Msk;  adc->SQR1 |= channel << ADC_SQR1_SQ2_Pos;  break;
		case 2:  adc->SQR1 &= ~ADC_SQR1_SQ3_Msk;  adc->SQR1 |= channel << ADC_SQR1_SQ3_Pos;  break;
		case 3:  adc->SQR1 &= ~ADC_SQR1_SQ4_Msk;  adc->SQR1 |= channel << ADC_SQR1_SQ4_Pos;  break;
		case 4:  adc->SQR2 &= ~ADC_SQR2_SQ5_Msk;  adc->SQR2 |= channel << ADC_SQR2_SQ5_Pos;  break;
		case 5:  adc->SQR2 &= ~ADC_SQR2_SQ6_Msk;  adc->SQR2 |= channel << ADC_SQR2_SQ6_Pos;  break;
		case 6:  adc->SQR2 &= ~ADC_SQR2_SQ7_Msk;  adc->SQR2 |= channel << ADC_SQR2_SQ7_Pos;  break;
		case 7:  adc->SQR2 &= ~ADC_SQR2_SQ8_Msk;  adc->SQR2 |= channel << ADC_SQR2_SQ8_Pos;  break;
		case 8:  adc->SQR2 &= ~ADC_SQR2_SQ9_Msk;  adc->SQR2 |= channel << ADC_SQR2_SQ9_Pos;  break;
		case 9:  adc->SQR3 &= ~ADC_SQR3_SQ10_Msk; adc->SQR3 |= channel << ADC_SQR3_SQ10_Pos; break;
		case 10: adc->SQR3 &= ~ADC_SQR3_SQ11_Msk; adc->SQR3 |= channel << ADC_SQR3_SQ11_Pos; break;
		case 11: adc->SQR3 &= ~ADC_SQR3_SQ12_Msk; adc->SQR3 |= channel << ADC_SQR3_SQ12_Pos; break;
		case 12: adc->SQR3 &= ~ADC_SQR3_SQ13_Msk; adc->SQR3 |= channel << ADC_SQR3_SQ13_Pos; break;
		case 13: adc->SQR3 &= ~ADC_SQR3_SQ14_Msk; adc->SQR3 |= channel << ADC_SQR3_SQ14_Pos; break;
		case 14: adc->SQR4 &= ~ADC_SQR4_SQ15_Msk; adc->SQR4 |= channel << ADC_SQR4_SQ15_Pos; break;
		case 15: adc->SQR4 &= ~ADC_SQR4_SQ16_Msk; adc->SQR4 |= channel << ADC_SQR4_SQ16_Pos; break;
		default: return -1;
	}

	adc->SQR1 &= ~ADC_SQR1_L_Msk;
	adc->SQR1 |= ((total_positions_count - 1U) & ADC_SQR1_L_Msk);

	return 0;
}

uint32_t adc_read(ADC_TypeDef *adc, uint32_t position)
{
	return adc->DR;
}
