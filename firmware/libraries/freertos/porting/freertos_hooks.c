
#include "FreeRTOS.h"
#include "task.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
//	DEBUG_PrintERROR("Stack overflow! Task handler: %p, Task name: %s\r\n", xTask, pcTaskName);
	while(1);
}
