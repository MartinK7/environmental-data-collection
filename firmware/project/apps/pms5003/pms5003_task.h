

#ifndef PROJECT_APPS_PMS5003_H_
#define PROJECT_APPS_PMS5003_H_

#include <stdint.h>

struct pms5003_data {
	union {
		float raw[12];
		struct {
			float std_pm1_0_ug_m3;
			float std_pm2_5_ug_m3;
			float std_pm10_ug_m3;
			float atm_pm1_ug_m3;
			float atm_pm2_5_ug_m3;
			float atm_pm10_ug_m3;
			float count_0_3_um_in_100cm3;
			float count_0_5_um_in_100cm3;
			float count_1_0_um_in_100cm3;
			float count_2_5_um_in_100cm3;
			float count_5_0_um_in_100cm3;
			float count_10_0_um_in_100cm3;
		};
	};
};

void pms5003_task_initialise(void);
int8_t pms5003_get_data(struct pms5003_data *data);
int8_t pms5003_power_on(void);
int8_t pms5003_power_off(void);

#endif /* PROJECT_APPS_PMS5003_H_ */
