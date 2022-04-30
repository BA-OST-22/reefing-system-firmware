/*
 * state_transition.c
 *
 *  Created on: 8 Apr 2022
 *      Author: Luca
 */

#include "state_transition.h"
#include "config/config.h"
#include "config/globals.h"
#include "drivers/led.h"
#include "drivers/sleep.h"
#include "init/init.h"
#include <tasks/definition.h>

/* Transition Deep Sleep -> Idle */
void state_transition_deepsleep_idle() {
  osEventFlagsSet(buzzer_event_id, BEEP_OK);
  osThreadNew(task_supervision, NULL, &task_supervision_attributes);
  init_idle();
  led_off();
}

/* Transition Idle -> Deep Sleep */
void state_transition_idle_deepsleep() {
  osEventFlagsSet(buzzer_event_id, BEEP_NOK);
  osEventFlagsSet(supervisor_mode_id, SUPERVISOR_MODE_SHUTDOWN);
  /* Give it some time to beep */
  osDelay(100);
  init_deepsleep();
  go_to_sleep(WAKEUP_BUTTON);
}

/* Transition Idle -> Ready */
void state_transition_idle_ready() {
  osThreadNew(task_sensor_read, NULL, &task_sensor_read_attributes);
  osThreadNew(task_state_est, NULL, &task_state_est_attributes);
  if (global_config.config.enable_telemetry) {
    osThreadNew(task_telemetry, NULL, &task_telemetry_attributes);
  }
  osEventFlagsSet(buzzer_event_id, BEEP_READY);
}

/* Transition Ready -> Idle */
void state_transition_ready_idle() {
  led_off();
  osEventFlagsSet(sensor_mode_id, SENSOR_MODE_SHUTDOWN);
  if (global_config.config.enable_telemetry) {
    osEventFlagsSet(telemetry_mode_id, TELEMETRY_MODE_SHUTDOWN);
  }
  osEventFlagsSet(state_est_mode_id, STATE_EST_MODE_SHUTDOWN);
  osEventFlagsSet(buzzer_event_id, BEEP_NOT_READY);
}

/* Transition Ready -> Ascent */
void state_transition_ready_ascent() {
  recorder_flush(&recorder);
  osEventFlagsSet(buzzer_event_id, BEEP_OK);
  osThreadNew(task_heater, NULL, &task_heater_attributes);
  if (global_config.config.enable_preheat) {
    osEventFlagsSet(heater_mode_id, HEATER_MODE_PREHEAT);
  } else {
    osEventFlagsSet(heater_mode_id, HEATER_MODE_IDLE);
  }
}

/* Transition Ascent -> Descent*/
void state_transition_ascent_descent() {
  osEventFlagsSet(buzzer_event_id, BEEP_OK);
}

/* Transition Descent -> Deployment */
void state_transition_descent_deployment() {
  osEventFlagsSet(buzzer_event_id, BEEP_OK);
  osEventFlagsSet(heater_mode_id, HEATER_MODE_HEAT);
}

/* Transition Deployment -> Recovery */
void state_transition_deployment_recovery() {
  osEventFlagsSet(buzzer_event_id, BEEP_OK);
  osEventFlagsSet(heater_mode_id, HEATER_MODE_SHUTDOWN);
}

/* Transition Recovery -> Idle */
void state_transition_recovery_idle() {
  osEventFlagsSet(sensor_mode_id, SENSOR_MODE_SHUTDOWN);
  if (global_config.config.enable_telemetry) {
    osEventFlagsSet(telemetry_mode_id, TELEMETRY_MODE_SHUTDOWN);
  }
  osEventFlagsSet(state_est_mode_id, STATE_EST_MODE_SHUTDOWN);
  osEventFlagsSet(buzzer_event_id, BEEP_NOT_READY);
}
