
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "i2c.h"

#include "../Adafruit_SI1145_Library/Adafruit_SI1145.h"

void si1145_delay(uint32_t delay_ms)
{
	vTaskDelay(delay_ms / portTICK_PERIOD_MS);
}

int8_t si1145_i2c_init(si1145_t *si1145)
{
	return 0;
}

void si1145_i2c_write_then_read(si1145_t *si1145, const uint8_t *buffer_tx, int buffer_tx_count, uint8_t *buffer_rx, int buffer_rx_count)
{
	uint8_t buffer[8];
	if(buffer_tx_count > sizeof(buffer)) {
		return;// -1;
	}
	memcpy(&buffer[0], buffer_tx, buffer_tx_count);

	if(i2c_master_transaction(si1145->dev_i2c, si1145->addr, I2C_DIRECTION_WRITE, buffer, buffer_tx_count)) {
		return;// -1;
	}

	if(i2c_master_transaction(si1145->dev_i2c, si1145->addr, I2C_DIRECTION_READ, buffer_rx, buffer_rx_count)) {
		return;// -1;
	}

	return;// 0;
}

void si1145_i2c_write(si1145_t *si1145, const uint8_t *buffer_tx, int buffer_tx_count)
{
	uint8_t buffer[8];
	if(buffer_tx_count > sizeof(buffer)) {
		return;// -1;
	}
	memcpy(&buffer[0], buffer_tx, buffer_tx_count);

	if(i2c_master_transaction(si1145->dev_i2c, si1145->addr, I2C_DIRECTION_WRITE, buffer, buffer_tx_count)) {
		return;// -1;
	}

	return;// 0;
}
