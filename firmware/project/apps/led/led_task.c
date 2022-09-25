
#include "led_task.h"

#include "gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lwip/netif.h"

static void led_task(void *parameters)
{
	for(;;)
	{
		gpio_pin_toggle(GPIO_PIN_USER_LED);

#if !defined(NO_ETH)
		extern struct netif gnetif;
		vTaskDelay((gnetif.ip_addr.addr == 0 ? 500 : 100) / portTICK_PERIOD_MS);
#else
		vTaskDelay(500 / portTICK_PERIOD_MS);
#endif // !NO_ETH
	}
}

void led_task_initialise(void)
{
	#define STACK_SIZE_LED 128
	static StaticTask_t xTaskBuffer;
	static StackType_t xStack[STACK_SIZE_LED];
	xTaskCreateStatic(led_task, "LED", STACK_SIZE_LED, NULL, 1, xStack, &xTaskBuffer);
}
