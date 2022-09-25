
#ifndef LIBRARIES_ESP32_ESP32_H_
#define LIBRARIES_ESP32_ESP32_H_

#include <stdint.h>
#include "wifi_over_spi.h"

int8_t esp32_init(const char *ssid, const char *password);

int8_t esp32_deinit(void);

int8_t esp32_tcp_open(const char *url, uint16_t port);
int8_t esp32_tcp_write(const uint8_t *buffer_tx, uint32_t count_tx);
int8_t esp32_tcp_close(void);

#endif /* LIBRARIES_ESP32_ESP32_H_ */
