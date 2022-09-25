
#ifndef DRIVERS_SPI_H_
#define DRIVERS_SPI_H_

#include "stm32h7xx.h"

int8_t spi_master_init(SPI_TypeDef *spi, uint32_t baudrate);
int8_t spi_master_deinit(SPI_TypeDef *spi);
int8_t spi_master_transaction(SPI_TypeDef *spi, const uint8_t *buffer_tx, uint16_t count_tx, uint8_t *buffer_rx, uint16_t count_rx);

#endif /* DRIVERS_SPI_H_ */
