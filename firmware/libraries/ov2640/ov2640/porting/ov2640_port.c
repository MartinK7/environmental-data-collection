
#include "ov2640.h"

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"
#include "i2c.h"

#define CAMERA_I2C I2C1

void OV2640_GPIOResetPinWrite(uint8_t value)
{
	gpio_pin_write(GPIO_PIN_CAM_RESET, value ? GPIO_PIN_STATE_HIGH : GPIO_PIN_STATE_LOW);
}

void OV2640_DelayMs(uint32_t DelayMs)
{
	vTaskDelay(DelayMs / portTICK_PERIOD_MS);
}

int OV2640_I2CMasterInit(void)
{
	return i2c_init_master(CAMERA_I2C, I2C_TIMING_STANDARD_100KHZ);
}

int OV2640_I2CMasterTransmit(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	if(i2c_master_transaction(CAMERA_I2C, DevAddress >> 1, I2C_DIRECTION_WRITE, pData, Size)) {
		return -1;
	}
	return 0;
}

int OV2640_I2CMasterReceive(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	if(i2c_master_transaction(CAMERA_I2C, DevAddress >> 1, I2C_DIRECTION_READ, pData, Size)) {
		return -1;
	}
	return 0;
}
