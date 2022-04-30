/*
 * definition.c
 *
 *  Created on: 12.04.2022
 *      Author: luca.jost
 */

#include "tasks/definition.h"

#define SET_TASK_PARAMS(task, stack_sz)                                        \
  uint32_t task##_buffer[stack_sz];                                            \
  StaticTask_t task##_control_block;                                           \
  const osThreadAttr_t task##_attributes = {                                   \
      .name = #task,                                                           \
      .stack_mem = &task##_buffer[0],                                          \
      .stack_size = sizeof(task##_buffer),                                     \
      .cb_mem = &task##_control_block,                                         \
      .cb_size = sizeof(task##_control_block),                                 \
      .priority = (osPriority_t)osPriorityNormal,                              \
  };

SET_TASK_PARAMS(task_sensor_read, 512)
SET_TASK_PARAMS(task_fsm, 256)
SET_TASK_PARAMS(task_state_est, 512)
SET_TASK_PARAMS(task_heater, 256)
SET_TASK_PARAMS(task_buzzer, 128)
SET_TASK_PARAMS(task_cli, 256)
SET_TASK_PARAMS(task_supervision, 256)
SET_TASK_PARAMS(task_telemetry, 256)
