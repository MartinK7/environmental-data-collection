
#ifndef DRIVERS_I2C_H_
#define DRIVERS_I2C_H_

#include "stm32h7xx.h"

enum i2c_direction {
	I2C_DIRECTION_WRITE = 0,
	I2C_DIRECTION_READ = 1
};

enum i2c_timing {
	I2C_TIMING_STANDARD_10KHZ = 0,
	I2C_TIMING_STANDARD_100KHZ,
	I2C_TIMING_FAST_400KHZ,
	I2C_TIMING_FAST_1000KHZ
};

int8_t i2c_init_master(I2C_TypeDef *i2c, enum i2c_timing timing);
int8_t i2c_master_transaction(I2C_TypeDef *i2c, uint8_t address_7bit, enum i2c_direction operation, uint8_t *buffer, uint8_t count);

#endif /* DRIVERS_I2C_H_ */
