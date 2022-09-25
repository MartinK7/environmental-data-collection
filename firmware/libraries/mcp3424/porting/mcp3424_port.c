
#include "mcp3424.h"

#include <stdlib.h>
#include <string.h>

#include "i2c.h"

int32_t mcp3424_port_i2c_master_transmit(struct mcp3424 *d, const uint8_t *txbuf, uint8_t txbytes, uint8_t *rxbuf, uint8_t rxbytes)
{
	if(txbuf != NULL && rxbuf != NULL) {
		return MCP3424_PORT_I2C_MASTER_TRANSMIT_FAILED;
	}

	if(txbuf != NULL) {
		// Write
		uint8_t buffer[8];
		if(txbytes > sizeof(buffer)) {
			return MCP3424_PORT_I2C_MASTER_TRANSMIT_FAILED;
		}
		memcpy(buffer, txbuf, txbytes);
		if(i2c_master_transaction(d->dev_i2c, d->addr, I2C_DIRECTION_WRITE, buffer, txbytes)) {
			return MCP3424_PORT_I2C_MASTER_TRANSMIT_FAILED;
		}
		return MCP3424_PORT_I2C_MASTER_TRANSMIT_OK;
	}

	// Read
	if(i2c_master_transaction(d->dev_i2c, d->addr, I2C_DIRECTION_READ, rxbuf, rxbytes)) {
		return MCP3424_PORT_I2C_MASTER_TRANSMIT_FAILED;
	}
	return MCP3424_PORT_I2C_MASTER_TRANSMIT_OK;
}
