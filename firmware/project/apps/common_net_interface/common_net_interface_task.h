
#ifndef PROJECT_APPS_NET_INTERACE_TASK_H_
#define PROJECT_APPS_NET_INTERACE_TASK_H_

#include <stdint.h>

#define CNI_UPLOAD_DATA_MAX_SIZE 1024

void cni_task_initialise(void);

int8_t cni_upload_timestamped_data(const uint8_t *timestamped_data, uint32_t data_size);

int8_t cni_upload_timestamped_data_blocking(const uint8_t *data, uint32_t data_size, uint32_t timeout);

#endif /* PROJECT_APPS_NET_INTERACE_TASK_H_ */
