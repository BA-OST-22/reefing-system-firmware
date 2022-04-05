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

void task_heater(void *argument) {

	int32_t tick_count = osKernelGetTickCount();
	uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;

	while (1) {


		dcdc_set_voltage(8);
		dcdc_enable();

		tick_count += tick_update;
		osDelayUntil(tick_count);
	}
}
