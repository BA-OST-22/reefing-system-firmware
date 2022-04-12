/*
 * task_telemetry.c
 *
 *  Created on: 12.04.2022
 *      Author: luca.jost
 */

#include "cmsis_os.h"
#include "tasks/task_telemetry.h"
#include "config/globals.h"
#include "util/log.h"

void task_telemetry(void *argument) {

	uint32_t tick_count = osKernelGetTickCount();
	uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;
	while (1) {


		tick_count += tick_update;
		osDelayUntil(tick_count);
	}
}
