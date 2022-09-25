
#include "measure_task.h"

#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "spi.h"
#include "i2c.h"
#include "gpio.h"
#include "Adafruit_SI1145.h"
#include "systick.h"
#include "usart.h"
#include "driver_bh1750fvi_shot.h"
#include "driver_bh1750fvi_basic.h"
#include "mcp3424.h"
#include "stmflash_storage.h"
#include "../pms5003/pms5003_task.h"
#include "../camera/camera_task.h"
#include "../mechanic/mechanic_task.h"

#include "SensorsSDK/WeSensorsSDK.h"
#include "SensorsSDK/WSEN_PADS_2511020213301/WSEN_PADS_2511020213301.h"
#include "WSEN_PADS_ADVANCED_EXAMPLE.h"

#include "../net/net_task.h"
#include "../common_net_interface/common_net_interface_task.h"

// I2C4
// (0x5D - 93) ABSOLUTE PRESSURE SENSOR                 https://www.we-online.com/components/products/manual/2511020213301_WSEN-PADS%202511020213301%20Manual_rev2.2.pdf
// (0x44!- 68) Relative Humidity and Temperature Sensor https://cz.mouser.com/datasheet/2/682/Datasheet_SHT4x-3003109.pdf

// I2C2
// (0x23 - 35) Ambient Light Sensor IC                  https://cz.mouser.com/datasheet/2/348/Rohm_11162017_ROHMS34826-1-1279292.pdf
// (0x60 - 96) PROXIMITY/UV/AMBIENT LIGHT SENSOR        https://cz.mouser.com/datasheet/2/737/Si1145-46-47-932790.pdf
// (0x68 -104) MCP3424T-E/SL 4-Channel Single ADC       https://static6.arrow.com/aropdfconversion/4eb73eb13f158d9d3cdfc3609d0f121d7f0202de/22088c.pdf
// (  |-->CH1) MQ131-LOW Ozone                          https://www.tme.eu/Document/6e5b194c14be0dabfc05afc0617ee35c/mq131low.pdf
// (  |-->CH2) 5V power supply calibration
// (  |-->CH3) CO sensor                                https://cz.mouser.com/datasheet/2/18/0278_Datasheet-MiCS-4514-rev-16-1144833.pdf
// (  |-->CH4) NOX sensor                               -||-

/// Build message

static struct __attribute__((packed, aligned(4))) {
	uint32_t timestamp;
	char measure_buffer[1024];
} message;

static struct stmflash_storage_databock settings;

static void measures_add_result(const char *name, const char *unit, float value)
{
	printf("New measure: %s %f %s\r\n", name, value, unit);

	size_t len = strlen(message.measure_buffer);
	int ret = snprintf(message.measure_buffer + len, sizeof(message.measure_buffer) - len, "%s%s=%f", len == 0 ? "EDCASCII:" : ",", name, value);
	if(ret + 1 > sizeof(message.measure_buffer) - len) {
		// Error, overflow! Truncate...
		message.measure_buffer[len] = '\0';
	}
}

static void measures_clear(void)
{
	memset(&message, 0, sizeof(message));
}

/// Measure

static int measure_si1145(void)
{
	static int si1145_ready = 0;
	static si1145_t si1145;
	if(!si1145_ready) {
		if(!(si1145_init(&si1145, 0x60, I2C2))) {
			return 1;
		}
		si1145_ready = 1;
	}

	si1145_delay(1000);

	measures_add_result("si1145_visible", "raw", (float)si1145_readVisible(&si1145));
	measures_add_result("si1145_ir", "raw", (float)si1145_readIR(&si1145));

	// 1 unit == 25mW/m^2 reduced std skin
	measures_add_result("si1145_uv_index", "UV Index", (float)si1145_readUV(&si1145) / 100.0f);

    return 0;
}

static int measure_bh1750fvi(void)
{
	// I2C2
	float    bh1750fvi_lux;

	float tmp, tmpS = 0.0f;
	for(int i = 0; i < 4; ++i) {
		if(bh1750fvi_shot_init(0x23)) {
			return -1;
		}
		if(bh1750fvi_shot_read(&tmp)) {
			return -2;
		}
		tmpS += tmp;
	}
	bh1750fvi_lux = tmpS / (float)4;

	measures_add_result("bh1750fvi_lux", "lx", bh1750fvi_lux);
	return 0;
}


static int measure_wsen2511020213301(void)
{
	// I2C4
	uint32_t wsen2511020213301_pressure;
	int32_t  wsen2511020213301_temperature;

	if(WE_padsAdvancedExampleInit()) {
		return -1;
	}

	if(PADS_startContinuousExample(&wsen2511020213301_pressure, &wsen2511020213301_temperature)) {
		return -2;
	}

	measures_add_result("wsen2511020213301_pressure", "hPa", (float)wsen2511020213301_pressure / 100.0f);
	measures_add_result("wsen2511020213301_temperature", "°C", (float)wsen2511020213301_temperature / 100.0f);
	return 0;
}

static int measure_mcp3424t(void)
{
	// I2C2
	int32_t  mcp3424t_adc[4];

	struct mcp3424 d;
	if(mcp3424_init(&d, I2C2, 0x68)) {
		return -1;
	}
	if(mcp3424_set_mode(&d, MCP3424_MODE_SINGLE_SHOT)) {
		return -1;
	}

	for(uint8_t channel = 0; channel < 4; ++channel) {
		if(mcp3424_set_channel(&d, channel)) {
			return -1;
		}
		if(mcp3424_set_gain(&d, MCP3424_GAIN_1X)) {
			return -1;
		}
		if(mcp3424_set_sample_rate(&d, MCP3424_RATE_3_75)) {
			return -1;
		}
		if(mcp3424_convert(&d)) {
			return -1;
		}
		uint32_t timeout = systick_get();
		while(mcp3424_check_result(&d) != MCP3424_CHECK_RESULT_UPDATED) {
			if(systick_check_timeout(timeout, 2000)) {
				return -2;
			}
			vTaskDelay(50 / portTICK_PERIOD_MS);
		}
		mcp3424_read_result(&d, &mcp3424t_adc[channel]);
	}

	// R = Rx+Ru+Rd
	// I = U / R
	stmflash_storage_read(&settings);
	float ref5V = (2.048f / 131071.0f * (float)mcp3424t_adc[1]) *
			(settings.gases.resistor_5Vref_up + settings.gases.resistor_5Vref_down) /
			settings.gases.resistor_5Vref_down;
	measures_add_result("mcp3424t_5V_calibration", "V", ref5V);

	#define Rx(Ud, Ru, Rd) (((Rd) * (ref5V) - (Ud) * ((Ru) + (Rd))) / (Ud))
	measures_add_result("mcp3424t_mq131_O3", "Ohm",
		Rx(2.048f / 131071.0f * (float)mcp3424t_adc[0], settings.gases.resistor_o3_up, settings.gases.resistor_o3_down)
	);
	measures_add_result("mcp3424t_mics4514_CO", "uS",
		1000000.0f / Rx(2.048f / 131071.0f * (float)mcp3424t_adc[2], settings.gases.resistor_co_up, settings.gases.resistor_co_down)
	);
	measures_add_result("mcp3424t_mics4514_NOx", "Ohm",
		Rx(2.048f / 131071.0f * (float)mcp3424t_adc[3], settings.gases.resistor_nox_up, settings.gases.resistor_nox_down)
	);

	return 0;
}

static int measure_pms5003(void)
{
	struct pms5003_data data;
	if(pms5003_get_data(&data)) {
		return -1;
	}
	measures_add_result("std_pm1_0_ug_m3", "ug/m3", data.std_pm1_0_ug_m3);
	measures_add_result("std_pm2_5_ug_m3", "ug/m3", data.std_pm2_5_ug_m3);
	measures_add_result("std_pm10_ug_m3", "ug/m3", data.std_pm10_ug_m3);
	measures_add_result("atm_pm1_ug_m3", "ug/m3", data.atm_pm1_ug_m3);
	measures_add_result("atm_pm2_5_ug_m3", "ug/m3", data.atm_pm2_5_ug_m3);
	measures_add_result("atm_pm10_ug_m3", "ug/m3", data.atm_pm10_ug_m3);
	measures_add_result("count_0_3_um_in_100cm3", "-", data.count_0_3_um_in_100cm3);
	measures_add_result("count_0_5_um_in_100cm3", "-", data.count_0_5_um_in_100cm3);
	measures_add_result("count_1_0_um_in_100cm3", "-", data.count_1_0_um_in_100cm3);
	measures_add_result("count_2_5_um_in_100cm3", "-", data.count_2_5_um_in_100cm3);
	measures_add_result("count_5_0_um_in_100cm3", "-", data.count_5_0_um_in_100cm3);
	measures_add_result("count_10_0_um_in_100cm3", "-", data.count_10_0_um_in_100cm3);
	return 0;
}

static void measurenet_task(void *parameter)
{
	// Initialize mechanical sensors
	mechanic_task_initialise();

	// Start measuring particles
	pms5003_task_initialise();

	// Power on
	gpio_pin_write(GPIO_PIN_CON_I2C2_3V3_EN, GPIO_PIN_STATE_HIGH);
	gpio_pin_write(GPIO_PIN_CON_I2C2_5V_EN, GPIO_PIN_STATE_HIGH);
	gpio_pin_write(GPIO_PIN_CON_I2C4_3V3_EN, GPIO_PIN_STATE_HIGH);
	pms5003_power_on();
	vTaskDelay(500 / portTICK_PERIOD_MS);

	// Camera OV2640
	camera_task_initialise();

	while(1) {
		// Clear result table
		measures_clear();

		// Light sensors
		measure_si1145();
		measure_bh1750fvi();

		// Gas sensors
		measure_mcp3424t();

		// Particles
		measure_pms5003();

		// Mechanical sensors
		//TODO

		// External sensors
		if(measure_wsen2511020213301()) {
			gpio_pin_write(GPIO_PIN_CON_I2C4_3V3_EN, GPIO_PIN_STATE_LOW);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			gpio_pin_write(GPIO_PIN_CON_I2C4_3V3_EN, GPIO_PIN_STATE_HIGH);
		}

		// Bind timestamp
		message.timestamp = systick_get();

		// Commit data
		cni_upload_timestamped_data((const uint8_t*)&message, sizeof(message.timestamp) + strlen(message.measure_buffer));

		// Small delay
		vTaskDelay(60 * 1000 / portTICK_PERIOD_MS);
	}

	// Power off
	vTaskDelay(500 / portTICK_PERIOD_MS);
	gpio_pin_write(GPIO_PIN_CON_I2C4_3V3_EN, GPIO_PIN_STATE_LOW);
	gpio_pin_write(GPIO_PIN_CON_I2C2_3V3_EN, GPIO_PIN_STATE_LOW);
	gpio_pin_write(GPIO_PIN_CON_I2C2_5V_EN, GPIO_PIN_STATE_LOW);
	pms5003_power_off();
}

void measurenet_task_initialise(void)
{
	#define STACK_SIZE 1024
	static StaticTask_t xTaskBuffer;
	static StackType_t xStack[STACK_SIZE];
	xTaskCreateStatic(measurenet_task, "MEASURE", STACK_SIZE, NULL, 1, xStack, &xTaskBuffer);
}
