
#ifndef DRIVERS_DCMI_H_
#define DRIVERS_DCMI_H_

#include <stdint.h>

int8_t dcmi_init(void);
uint32_t dcmi_capture(uint32_t *buffer, uint32_t buffer_size_words);

#endif /* DRIVERS_DCMI_H_ */
