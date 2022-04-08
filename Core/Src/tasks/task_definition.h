#pragma once

#include "cmsis_os.h"

#include "tasks/task_buzzer.h"
#include "tasks/task_fsm.h"
#include "tasks/task_heater.h"
#include "tasks/task_sensor_read.h"
#include "tasks/task_state_est.h"
#include "tasks/task_cli.h"

#define SET_TASK_PARAMS(task, stack_sz)           \
  uint32_t task##_buffer[stack_sz];               \
  StaticTask_t task##_control_block;              \
  const osThreadAttr_t task##_attributes = {      \
      .name = #task,                              \
      .stack_mem = &task##_buffer[0],             \
      .stack_size = sizeof(task##_buffer),        \
      .cb_mem = &task##_control_block,            \
      .cb_size = sizeof(task##_control_block),    \
      .priority = (osPriority_t)osPriorityNormal, \
  };

SET_TASK_PARAMS(task_sensor_read, 512)
SET_TASK_PARAMS(task_fsm, 512)
SET_TASK_PARAMS(task_state_est, 512)
SET_TASK_PARAMS(task_heater, 256)
SET_TASK_PARAMS(task_buzzer, 256)
SET_TASK_PARAMS(task_cli, 256)

osThreadId_t task_sensor_read_handle;
osThreadId_t task_fsm_handle;
osThreadId_t task_state_est_handle;
osThreadId_t task_heater_handle;
osThreadId_t task_buzzer_handle;
osThreadId_t task_cli_handle;
