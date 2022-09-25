
#ifndef DRIVERS_STMFLASH_H_
#define DRIVERS_STMFLASH_H_

#include <stdint.h>

struct stmflash_storage_databock {
	/// Sign 1
	uint32_t data_valid1;

	/// Data
	struct {
		char     url[32];
		uint16_t port;
	} server;

	struct {
		uint8_t enable;
	} ethernet;

	struct {
		uint8_t enable;
		char    ssid[64];
		char    password[64];
	} wifi;

	struct {
		uint8_t enable;
		char    apn[32];
	} gsmlte;

	struct {
		uint32_t resistor_nox_up;
		uint32_t resistor_nox_down;
		uint32_t resistor_co_up;
		uint32_t resistor_co_down;
		uint32_t resistor_o3_up;
		uint32_t resistor_o3_down;
		uint32_t resistor_5Vref_up;
		uint32_t resistor_5Vref_down;
	} gases;

	/// Sign 2
	uint32_t data_valid2;
};

int8_t stmflash_storage_program(const struct stmflash_storage_databock *data);
int8_t stmflash_storage_read(struct stmflash_storage_databock *data);

#endif /* DRIVERS_STMFLASH_H_ */
