#pragma once

#include "cmsis_os.h"

#include "tasks/task_buzzer.h"
#include "tasks/task_fsm.h"
#include "tasks/task_heater.h"
#include "tasks/task_sensor_read.h"
#include "tasks/task_state_est.h"
#include "tasks/task_cli.h"
#include "tasks/task_supervision.h"
#include "tasks/task_telemetry.h"

extern const osThreadAttr_t task_buzzer_attributes;
extern const osThreadAttr_t task_cli_attributes;
extern const osThreadAttr_t task_fsm_attributes;
extern const osThreadAttr_t task_sensor_read_attributes;
extern const osThreadAttr_t task_state_est_attributes;
extern const osThreadAttr_t task_heater_attributes;
extern const osThreadAttr_t task_supervision_attributes;
extern const osThreadAttr_t task_telemetry_attributes;
