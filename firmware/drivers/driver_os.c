
#include "driver_os.h"

#include "FreeRTOS.h"
#include "task.h"

void driver_os_yield(void)
{
	if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
		taskYIELD();
	}
}
