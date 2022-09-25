
#ifndef DRIVERS_RCC_H_
#define DRIVERS_RCC_H_

#include <stdint.h>

#define RCC_READ_BITS(reg,name) (((reg) & (name##_Msk)) >> (name##_Pos))
#define RCC_WRITE_BITS(reg,name,value) do { (reg) = ((reg) | name##_Msk) & (~(value << name##_Pos)); } while(0)

void rcc_init(void);

uint32_t rcc_d1cpre_div_get(void);
uint32_t rcc_hpre_div_get(void);
uint32_t rcc_d3ppre_div_get(void);
uint32_t rcc_d2ppre1_div_get(void);
uint32_t rcc_d2ppre2_div_get(void);

uint32_t rcc_ahbx_get_clock(void);

uint32_t rcc_hsi_get_clock(void);

uint32_t rcc_apb1_get_clock(void);
uint32_t rcc_apb2_get_clock(void);
uint32_t rcc_apb1_timx_get_clock(void);
uint32_t rcc_apb2_timx_get_clock(void);
uint32_t rcc_apb4_get_clock(void);

#endif /* DRIVERS_RCC_H_ */
