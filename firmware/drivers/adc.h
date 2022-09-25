
#ifndef DRIVERS_ADC_H_
#define DRIVERS_ADC_H_

#include "stm32h7xx.h"

int8_t adc1_and_adc2_init(void);
int8_t adc_attach_callback(ADC_TypeDef *adc, void(*adc_callback)(uint32_t adc_value));
int8_t adc_start(ADC_TypeDef *adc);
int8_t adc_stop(ADC_TypeDef *adc);
int8_t adc_configure_channel(ADC_TypeDef *adc, uint32_t position, uint32_t channel, uint32_t conversions_count);
uint32_t adc_read(ADC_TypeDef *adc, uint32_t position);

#endif /* DRIVERS_ADC_H_ */
