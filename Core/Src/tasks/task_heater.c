/*
 * task_heater.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */


#include "cmsis_os.h"
#include "task_heater.h"
#include "config/globals.h"
#include "drivers/dcdc.h"
#include "sensors/max6675.h"
#include "util/log.h"
#include "target/target.h"

#define SAMPLING_FREQ_HEATER 1 //Hz

void task_heater(void *argument) {

	HAL_GPIO_WritePin(P_EN_CUT_GPIO_Port, P_EN_CUT_Pin, GPIO_PIN_SET);

	uint32_t tick_count = osKernelGetTickCount();
	uint32_t tick_update = 5000; //osKernelGetTickFreq() / SAMPLING_FREQ_HEATER;
	while (1) {
		float temperature = 0;
		thermocouple_status_t status;

		status = get_temperature(&temperature);

		if(status != TMP_OK) {
			log_error("No thermocouple connected!");
		} else {
			log_info("Current thermocouple temperature: %ld", (int32_t)temperature);
		}
		//dcdc_set_voltage(8);
		//dcdc_enable();

		tick_count += tick_update;
		osDelayUntil(tick_count);
	}
}
