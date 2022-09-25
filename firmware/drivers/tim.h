
#ifndef DRIVERS_TIM_H_
#define DRIVERS_TIM_H_

#include "stm32h7xx.h"

int8_t tim_clock_generator_init(TIM_TypeDef *tim, uint8_t channel, uint32_t frequency);

int8_t tim_counter_init(TIM_TypeDef *tim, uint32_t ticks_per_second, uint16_t cnt_max, uint8_t reload_enable);
uint16_t tim_counter_get(TIM_TypeDef *tim);
int8_t tim_counter_restart(TIM_TypeDef *tim);

#endif /* DRIVERS_TIM_H_ */
