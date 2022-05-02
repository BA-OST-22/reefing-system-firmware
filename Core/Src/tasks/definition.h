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

#pragma once

#include "cmsis_os.h"

#include "tasks/task_buzzer.h"
#include "tasks/task_cli.h"
#include "tasks/task_fsm.h"
#include "tasks/task_heater.h"
#include "tasks/task_sensor_read.h"
#include "tasks/task_state_est.h"
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
