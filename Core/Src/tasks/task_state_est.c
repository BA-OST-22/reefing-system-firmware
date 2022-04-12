/*
 * task_state_est.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */

#include "cmsis_os.h"
#include "task_state_est.h"
#include "config/globals.h"
#include "util/log.h"

void task_state_est(void *argument) {

	uint32_t tick_count = osKernelGetTickCount();
	uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;
	osDelay(2000);
	while (1) {

		state_est_mode_e mode = osEventFlagsWait(state_est_mode_id, 0xFF, osFlagsWaitAny, 0);

		if(mode == STATE_EST_MODE_SHUTDOWN){
			osThreadTerminate(osThreadGetId());
		}

		tick_count += tick_update;
		osDelayUntil(tick_count);
	}
}
