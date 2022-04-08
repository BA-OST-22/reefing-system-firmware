/*
 * task_fsm.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */


#include "cmsis_os.h"
#include "task_fsm.h"

#include "config/globals.h"
#include "config/config.h"

#include "util/log.h"
#include "flight/fsm.h"

void task_fsm(void *argument) {
	fsm_t fsm;
	fsm.flight_state = IDLE;

	osEventFlagsSet(buzzer_event_id, BEEP_BOOTUP);


	uint32_t tick_count = osKernelGetTickCount();
	uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;
	while (1) {
		update_fsm(&fsm);
		tick_count += tick_update;
		osDelayUntil(tick_count);
	}
}
