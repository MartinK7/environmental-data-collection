/* Martin Krásl (universal I2C function calls) */

#include "si1145.h"

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "i2c.h"

IO_SPA_RC si1145_io_i2c_init(SI1145_DEV *si1145)
{
	return IO_SPA_OK;
}

IO_SPA_RC si1145_io_i2c_read(SI1145_DEV *si1145, uint8_t reg, uint8_t num_bytes, uint8_t *data)
{
	if(i2c_master_transaction(si1145->dev_i2c, si1145->addr, I2C_DIRECTION_WRITE, &reg, 1)) {
		return IO_SPA_ERROR;
	}

	if(i2c_master_transaction(si1145->dev_i2c, si1145->addr, I2C_DIRECTION_READ, data, num_bytes)) {
		return IO_SPA_ERROR;
	}

	return IO_SPA_OK;
}

IO_SPA_RC si1145_io_i2c_write(SI1145_DEV *si1145, uint8_t reg, uint8_t num_bytes, uint8_t *data)
{
	uint8_t buffer[8];
	if(num_bytes + 1 > sizeof(buffer)) {
		return IO_SPA_ERROR;
	}

	buffer[0] = reg;
	memcpy(&buffer[1], data, num_bytes);

	if(i2c_master_transaction(si1145->dev_i2c, si1145->addr, I2C_DIRECTION_WRITE, buffer, num_bytes + 1)) {
		return IO_SPA_ERROR;
	}

	return IO_SPA_OK;
}

IO_SPA_RC si1145_io_i2c_close(SI1145_DEV *si1145)
{
	return IO_SPA_OK;
}

IO_SPA_RC si1145_sleep(SI1145_DEV *si1145, uint8_t seconds)
{
	vTaskDelay(seconds * 1000U / portTICK_PERIOD_MS);
	return IO_SPA_OK;
}
