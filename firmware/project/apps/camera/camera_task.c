
#include "camera_task.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "task.h"

#include "usart.h"
#include "gpio.h"
#include "ov2640.h"
#include "tim.h"
#include "systick.h"
#include "dcmi.h"
#include "../common_net_interface/common_net_interface_task.h"

static enum imageResolution imgRes = RES_1280x960;
#define CAMERA_IMAGE_BUFFER (320 * 240 * 3)

static struct {
	// Header
	uint32_t timestamp;
	char edcbinary[10];
	uint16_t type;
	uint32_t size;
	uint32_t checksum;
	uint32_t pixels[CAMERA_IMAGE_BUFFER / sizeof(uint32_t)];
} binary_message  __attribute__((aligned(4), section(".ram_d1")));

static_assert((CAMERA_IMAGE_BUFFER & 3) == 0, "Must be aligned to 4 bytes!");

static void camera_task(void *parameters)
{
	usart_init(UART7, 115200);

	gpio_pin_write(GPIO_PIN_CAM_PWR_EN, GPIO_PIN_STATE_HIGH);
	OV2640_DelayMs(100);
	dcmi_init();
	tim_clock_generator_init(TIM3, 2, 24000000);
	OV2640_DelayMs(100);

	OV2640_Init();
	OV2640_DelayMs(10);
	OV2640_ResolutionOptions(imgRes);
	OV2640_DelayMs(100);

	for(;;) {
		memset(&binary_message, 0, sizeof(binary_message));
		memcpy(&binary_message.edcbinary, "EDCBINARY:", 10);
		binary_message.type = 0x0001U;
		binary_message.timestamp = systick_get();

		uint32_t image_size_words = dcmi_capture(binary_message.pixels, sizeof(binary_message.pixels) / sizeof(*binary_message.pixels));

		// Check DCMI error
		if(!image_size_words) {
			continue;
		}

		// Recode binary data
		uint8_t *p = (uint8_t *)binary_message.pixels;
		uint32_t image_size_bytes = 0;
		for(uint32_t i = 0; i < image_size_words; ++i) {
			uint32_t in = binary_message.pixels[i];
			*(p++) = (in >>  2U) & 0xFFU;
			*(p++) = (in >> 18U) & 0xFFU;
			image_size_bytes += 2;
		}

		uint32_t message_to_trasnsmit_size = sizeof(binary_message) + image_size_bytes - sizeof(binary_message.pixels);
		binary_message.size = message_to_trasnsmit_size - sizeof(binary_message.timestamp);

		uint32_t chksum = 0;
		for(uint32_t i = 0; i < binary_message.size; ++i) {
			chksum += (uint32_t)((const uint8_t*)&binary_message)[sizeof(binary_message.timestamp) + i];
		}
		binary_message.checksum = chksum;

		cni_upload_timestamped_data_blocking((const uint8_t*)&binary_message, message_to_trasnsmit_size, 60000);

		vTaskDelay(pdMS_TO_TICKS(1 * 60 * 1000));
	}
}

void camera_task_initialise(void)
{
	static StaticTask_t xTaskBuffer;
	static StackType_t xStack[1024];
	xTaskCreateStatic(camera_task, "CAMERA", sizeof(xStack) / sizeof(*xStack), NULL, 1, xStack, &xTaskBuffer);
}
