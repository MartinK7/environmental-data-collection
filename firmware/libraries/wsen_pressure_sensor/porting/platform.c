/*
 ***************************************************************************************************
 * This file is part of Sensors SDK:
 * https://www.we-online.com/sensors, https://github.com/WurthElektronik/Sensors-SDK_STM32
 *
 * THE SOFTWARE INCLUDING THE SOURCE CODE IS PROVIDED “AS IS”. YOU ACKNOWLEDGE THAT WÜRTH ELEKTRONIK
 * EISOS MAKES NO REPRESENTATIONS AND WARRANTIES OF ANY KIND RELATED TO, BUT NOT LIMITED
 * TO THE NON-INFRINGEMENT OF THIRD PARTIES’ INTELLECTUAL PROPERTY RIGHTS OR THE
 * MERCHANTABILITY OR FITNESS FOR YOUR INTENDED PURPOSE OR USAGE. WÜRTH ELEKTRONIK EISOS DOES NOT
 * WARRANT OR REPRESENT THAT ANY LICENSE, EITHER EXPRESS OR IMPLIED, IS GRANTED UNDER ANY PATENT
 * RIGHT, COPYRIGHT, MASK WORK RIGHT, OR OTHER INTELLECTUAL PROPERTY RIGHT RELATING TO ANY
 * COMBINATION, MACHINE, OR PROCESS IN WHICH THE PRODUCT IS USED. INFORMATION PUBLISHED BY
 * WÜRTH ELEKTRONIK EISOS REGARDING THIRD-PARTY PRODUCTS OR SERVICES DOES NOT CONSTITUTE A LICENSE
 * FROM WÜRTH ELEKTRONIK EISOS TO USE SUCH PRODUCTS OR SERVICES OR A WARRANTY OR ENDORSEMENT
 * THEREOF
 *
 * THIS SOURCE CODE IS PROTECTED BY A LICENSE.
 * FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE (license_terms_wsen_sdk.pdf)
 * LOCATED IN THE ROOT DIRECTORY OF THIS DRIVER PACKAGE.
 *
 * COPYRIGHT (c) 2022 Würth Elektronik eiSos GmbH & Co. KG
 *
 ***************************************************************************************************
 */

/**
 * @file
 * @brief Contains platform-specific functions.
 */

#include "platform.h"

#include <string.h>

#include "i2c.h"

/**
 * @brief Read data starting from the addressed register
 * @param[in] interface Sensor interface
 * @param[in] regAdr The register address to read from
 * @param[in] numBytesToRead Number of bytes to read
 * @param[out] data The read data will be stored here
 * @retval Error code
 */
inline int8_t WE_ReadReg(WE_sensorInterface_t *interface,
                         uint8_t regAdr,
                         uint16_t numBytesToRead,
                         uint8_t *data)
{
	if(i2c_master_transaction(interface->handle, 0x5D, I2C_DIRECTION_WRITE, &regAdr, 1)) {
		return WE_FAIL;
	}

	if(i2c_master_transaction(interface->handle, 0x5D, I2C_DIRECTION_READ, data, numBytesToRead)) {
		return WE_FAIL;
	}

	return WE_SUCCESS;
}


/**
 * @brief Write data starting from the addressed register
 * @param[in] interface Sensor interface
 * @param[in] regAdr Address of register to be written
 * @param[in] numBytesToWrite Number of bytes to write
 * @param[in] data Data to be written
 * @retval Error code
 */
inline int8_t WE_WriteReg(WE_sensorInterface_t *interface,
                          uint8_t regAdr,
                          uint16_t numBytesToWrite,
                          uint8_t *data)
{
	uint8_t buffer[8];
	if(numBytesToWrite + 1 > sizeof(buffer)) {
		return WE_FAIL;
	}

	buffer[0] = regAdr;
	memcpy(&buffer[1], data, numBytesToWrite);

	if(i2c_master_transaction(interface->handle, 0x5D, I2C_DIRECTION_WRITE, buffer, numBytesToWrite + 1)) {
		return WE_FAIL;
	}

	return WE_SUCCESS;
}

/**
 * @brief Checks if the sensor interface is ready.
 * @param[in] interface Sensor interface
 * @return WE_SUCCESS if interface is ready, WE_FAIL if not.
 */
int8_t WE_isSensorInterfaceReady(WE_sensorInterface_t* interface)
{
	return WE_SUCCESS;
}
