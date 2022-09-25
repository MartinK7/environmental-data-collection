
#ifndef LIBRARIES_A7670_A7670_H_
#define LIBRARIES_A7670_A7670_H_

#include <stdint.h>

int8_t a7670_init(const char *apn);

int8_t a7670_deinit(void);

int8_t a7670_tcp_transaction(const uint8_t *buffer_tx, uint16_t count_tx);

#endif /* LIBRARIES_A7670_A7670_H_ */
