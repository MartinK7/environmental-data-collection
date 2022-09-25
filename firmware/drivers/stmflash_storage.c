
#include "stmflash_storage.h"

#include <string.h>
#include "stm32h7xx.h"
#include "systick.h"

#define STMFLASH_STORAGE_DATA_VALID1 0xFEEDC0DEU
#define STMFLASH_STORAGE_DATA_VALID2 0xC0FFE777U

static const struct stmflash_storage_databock default_values = {
	.data_valid1 = STMFLASH_STORAGE_DATA_VALID1,
	.server = {
		.url = "pony.zapto.org",
		.port = 3002
	},
	.ethernet = {
		.enable = 0
	},
	.wifi = {
		.enable = 0,
		.ssid = "",
		.password = ""
	},
	.gsmlte = {
		.enable = 0,
		.apn = ""
	},
	.gases = {
		.resistor_nox_up = 10000,
		.resistor_nox_down = 6800,
		.resistor_co_up = 0,
		.resistor_co_down = 47000,
		.resistor_o3_up = 750000,
		.resistor_o3_down = 250000,
		.resistor_5Vref_up = 3900,
		.resistor_5Vref_down = 2200
	},
	.data_valid2 = STMFLASH_STORAGE_DATA_VALID2
};

// More storage instances increase wear leveling
// Better will be to have all 0xFF, but you know... C can't do that
static struct stmflash_storage_databock __attribute__((section(".stmflash_storage"))) storage_datablocks;

__STATIC_FORCEINLINE int8_t stmflash_storage_program_program_begin(uint32_t psize)
{
	uint32_t timeout = systick_get();

	while(FLASH->SR1 & FLASH_SR_BSY)
		if(systick_check_timeout(timeout, 1000))
			return -10;

	if(FLASH->SR1 & (FLASH_SR_OPERR | FLASH_SR_WRPERR /*| FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_ERSERR*/)) {
		return -11;
	}

	if(FLASH->SR1 & FLASH_SR_EOP)
		FLASH->SR1 = FLASH_SR_EOP;

	FLASH->CR1 &= ~FLASH_CR_PSIZE;
	FLASH->CR1 |= psize;
	FLASH->CR1 |= FLASH_CR_PG;

	return 0;
}

__STATIC_FORCEINLINE int8_t stmflash_storage_program_program_end(void)
{
	__DSB();

	FLASH->CR1 |= FLASH_CR_FW;

	uint32_t timeout = systick_get();

	while(FLASH->SR1 & FLASH_SR_BSY)
		if(systick_check_timeout(timeout, 1000))
			return -20;

	if(FLASH->SR1 & (FLASH_SR_OPERR | FLASH_SR_WRPERR /*| FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_ERSERR*/)) {
		return -21;
	}

	if(FLASH->SR1 & FLASH_SR_EOP)
		FLASH->CCR1 = FLASH_CCR_CLR_EOP;


	FLASH->CR1 &= ~FLASH_CR_PG;

	return 0;
}

__STATIC_FORCEINLINE int8_t stmflash_storage_erase(void)
{
	__DSB();

	uint32_t timeout = systick_get();
	uint32_t psize = 0;
	uint32_t sector = 7;

	FLASH->CR1 &= ~FLASH_CR_PSIZE;
	FLASH->CR1 |= psize;
	FLASH->CR1 &= ~FLASH_CR_SNB;
	FLASH->CR1 |= FLASH_CR_SER | (sector << FLASH_CR_SNB_Pos);
	FLASH->CR1 |= FLASH_CR_START;

	__DSB();
	while(FLASH->SR1 & FLASH_SR_BSY)
		if((systick_get() - timeout) > 0x10000U)
			return 2;

	if(FLASH->SR1 & (FLASH_SR_OPERR | FLASH_SR_WRPERR /*| FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_ERSERR*/)) {
		return 3;
	}

	if(FLASH->SR1 & FLASH_SR_EOP)
		FLASH->CCR1 = FLASH_CCR_CLR_EOP;

	FLASH->CR1 &= ~(FLASH_CR_SER | FLASH_CR_SNB);
	return 0;
}

// This function is not power safe if the erase is needed
int8_t stmflash_storage_program(const struct stmflash_storage_databock *data)
{
	int8_t retcode = -1;

	struct stmflash_storage_databock data_copy = *data;

	// Sign data
	data_copy.data_valid1 = STMFLASH_STORAGE_DATA_VALID1;
	data_copy.data_valid2 = STMFLASH_STORAGE_DATA_VALID2;

	// Find where to store data
	uint32_t perform_erase = 1;
	struct stmflash_storage_databock *program_ptr = &storage_datablocks;

	/// Unlock FLASH
	if(FLASH->CR1 & FLASH_CR_LOCK) {
		FLASH->KEYR1 = 0x45670123U;
		FLASH->KEYR1 = 0xCDEF89ABU;
	}

	if(FLASH->CR1 & FLASH_CR_LOCK) {
		retcode = -2;
		goto exit;
	}

	if(perform_erase) {
		/// Erase last sector (section ".stmflash_storage")
		stmflash_storage_erase();
	}

	retcode = stmflash_storage_program_program_begin(0);
	if(retcode)
		goto exit;

	/// Program loop
	for(uint32_t i = 0; i < sizeof(data_copy); ++i) {

		__DSB();
		*(((__IO uint8_t*)program_ptr) + i) = *(((__IO uint8_t*)&data_copy) + i);
		__DSB();
	}

	retcode = stmflash_storage_program_program_end();
	if(retcode)
		goto exit;

	/// Lock FLASH
	FLASH->CR1 |= FLASH_CR_LOCK;

	/// Check programmed data
	if(memcmp(program_ptr, &data_copy, sizeof(*program_ptr)) != 0) {
		retcode = -9;
		return retcode;
	}

	retcode = 0;
	return retcode;

exit:
	/// Lock FLASH
	FLASH->CR1 |= FLASH_CR_LOCK;

	// Return result
	return retcode;
}

int8_t stmflash_storage_read(struct stmflash_storage_databock *data)
{
	// Check latest saved data
	if(storage_datablocks.data_valid1 == STMFLASH_STORAGE_DATA_VALID1) {
		if(storage_datablocks.data_valid2 == STMFLASH_STORAGE_DATA_VALID2) {
			*data = storage_datablocks;
			return 0;
		}
	}
	// No settings found, return default values
	*data = default_values;
	// Program default settings
	stmflash_storage_program(&default_values);
	return 0;
}
