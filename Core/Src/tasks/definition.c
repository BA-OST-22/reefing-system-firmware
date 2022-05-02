/*
 * Reefing System Bachelor Thesis Software
 * Copyright (C) 2022 Institute for Microelectronics and Embedded Systems OST
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
