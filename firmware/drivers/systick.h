
#ifndef DRIVERS_SYSTICK_H_
#define DRIVERS_SYSTICK_H_

#include <stdint.h>

void systick_init(void);
uint32_t systick_get(void);
int8_t systick_check_timeout(uint32_t start_time_milliseconds, uint32_t timeout_milliseconds);
void systick_delay_milliseconds(uint32_t duration_milliseconds);
void systick_irq_add_callback(void(*cb)(void));

#endif /* DRIVERS_SYSTICK_H_ */
