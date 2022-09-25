

#ifndef PROJECT_APPS_TERMINAL_H_
#define PROJECT_APPS_TERMINAL_H_

#include <stdint.h>

void terminal_task_initialise(void);
void terminal_task_stdout_callback(const char *data, uint32_t size);

#endif /* PROJECT_APPS_TERMINAL_H_ */
