/*
 * state_transition.c
 *
 *  Created on: 8 Apr 2022
 *      Author: Luca
 */

#include <tasks/definition.h>
#include "state_transition.h"
#include "init/init.h"
#include "config/globals.h"
#include "config/config.h"
#include "drivers/led.h"
#include "drivers/sleep.h"

/* Transition Deep Sleep -> Idle */
void state_transition_deepsleep_idle(){
	osEventFlagsSet(buzzer_event_id, BEEP_OK);
	init_idle();
}

/* Transition Idle -> Deep Sleep */
void state_transition_idle_deepsleep(){
	osEventFlagsSet(buzzer_event_id, BEEP_NOK);
	/* Give it some time to beep */
	osDelay(100);
	init_deepsleep();
	go_to_sleep(WAKEUP_BUTTON);
}

/* Transition Idle -> Ready */
void state_transition_idle_ready(){
	osThreadNew(task_sensor_read, NULL, &task_sensor_read_attributes);
	osThreadNew(task_state_est, NULL, &task_state_est_attributes);
	if(global_config.config.use_telemetry){
		osThreadNew(task_telemetry, NULL, &task_telemetry_attributes);
	}
	osEventFlagsSet(buzzer_event_id, BEEP_READY);
}

/* Transition Ready -> Idle */
void state_transition_ready_idle(){
	osEventFlagsSet(sensor_mode_id, SENSOR_MODE_SHUTDOWN);
	osEventFlagsSet(state_est_mode_id, STATE_EST_MODE_SHUTDOWN);
	osEventFlagsSet(buzzer_event_id, BEEP_NOT_READY);
}

/* Transition Ready -> Ascent */
void state_transition_ready_ascent(){
	osThreadNew(task_heater, NULL, &task_heater_attributes);
}

/* Transition Ascent -> Descent*/
void state_transition_ascent_descent(){

}

/* Transition Descent -> Deployment */
void state_transition_descent_deployment(){

}

/* Transition Deployment -> Recovery */
void state_transition_deployment_recovery(){

}

/* Transition Recovery -> Deep Sleep */
void state_transition_recovery_deepsleep(){

}
