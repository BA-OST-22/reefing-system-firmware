/*
 * task_fsm.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */


#include "cmsis_os.h"
#include "task_fsm.h"
#include "config/globals.h"

void task_fsm(void *argument) {

	int32_t tick_count = osKernelGetTickCount();
	uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;

	while (1) {

		tick_count += tick_update;
		osDelayUntil(tick_count);
	}
}
