
#include "mechanic_task.h"

#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "tim.h"
#include "gpio.h"
#include "adc.h"

static SemaphoreHandle_t xSemaphore = NULL;

static volatile uint32_t isr_lock = 0;
static volatile float    max_windspeed_m_per_sec = 0.0f;
static volatile uint32_t wind_clicks = 0;
static volatile uint32_t gauge_clicks = 0;
static volatile uint32_t windir_counter[16] = {0};

static void gaue_isr(struct gpio_pin pin)
{
	if(isr_lock) {
		return;
	}
	++gauge_clicks;
}

static void anemometer_isr(struct gpio_pin pin)
{
	uint16_t tick = tim_counter_get(TIM1);
	tim_counter_restart(TIM1);

	if(isr_lock) {
		return;
	}

	++wind_clicks;

	if(tick) {
		float tpersec = 10000.0f / (float)tick;
		float windspeed_m_per_sec = tpersec * 2.4f / 3.6f;
		if(windspeed_m_per_sec > max_windspeed_m_per_sec) {
			max_windspeed_m_per_sec = windspeed_m_per_sec;
		}
	}
}

static void winddir_isr(uint32_t adc_value)
{
	if(isr_lock) {
		return;
	}
	//TODO ADC to wind direction
}

uint32_t aval = 0;

static void mechanic_task(void *parameters)
{
	gpio_pin_attach_interrupt(GPIO_PIN_GAUGE, GPIO_PIN_EDGE_FALLING, gaue_isr);
	gpio_pin_attach_interrupt(GPIO_PIN_ANEMOMETER, GPIO_PIN_EDGE_FALLING, anemometer_isr);

	tim_counter_init(TIM1, 10000, 0xFF00U, 1);

	adc1_and_adc2_init();
	adc_configure_channel(ADC1, 0, 15, 1);
	adc_attach_callback(ADC1, winddir_isr);
	adc_start(ADC1);

	for(;;) {
		isr_lock = 0;
		vTaskDelay(pdMS_TO_TICKS(5 * 60 * 1000));
		isr_lock = 1;
	}
}

int8_t mechanic_get_last_5mintes(float *wind_direction_degrees, float *water_liters_per_square_meter, float *average_wind_speed, float *gusts_wind_speed)
{
	if(!isr_lock) {
		return -1;
	}

	for(uint32_t i = 0, im = 0; i < sizeof(windir_counter) / sizeof(*windir_counter); ++i) {
		if(windir_counter[i] > im) {
			im = windir_counter[i];
			*wind_direction_degrees = (float)i * 360.0f / (float)(sizeof(windir_counter) / sizeof(*windir_counter));
		}
	}
	for(uint32_t i = 0; i < sizeof(windir_counter) / sizeof(*windir_counter); ++i) {
		 windir_counter[i] = 0;
	}

	*water_liters_per_square_meter = (float)gauge_clicks * 0.2794f;
	gauge_clicks = 0;

	*average_wind_speed = (float)wind_clicks / (float)(5 * 60) * 2.4f / 3.6f;
	wind_clicks = 0;

	*gusts_wind_speed = max_windspeed_m_per_sec;
	max_windspeed_m_per_sec = 0.0f;

	isr_lock = 0;
	return 0;
}

void mechanic_task_initialise(void)
{
	static StaticSemaphore_t xMutexBuffer;
	xSemaphore = xSemaphoreCreateMutexStatic(&xMutexBuffer);
	configASSERT(xSemaphore);

	#define STACK_SIZE_MECHANIC 128
	static StaticTask_t xTaskBuffer;
	static StackType_t xStack[STACK_SIZE_MECHANIC];
	xTaskCreateStatic(mechanic_task, "MECHANIC", STACK_SIZE_MECHANIC, NULL, 1, xStack, &xTaskBuffer);
}
