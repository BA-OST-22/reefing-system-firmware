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

#include "config/globals.h"
#include "stdbool.h"
#include "stdint.h"

typedef struct {
  fsm_state_e flight_state;
  uint32_t memory[4];
  bool state_changed;
  uint32_t state_change_time;
} fsm_t;

void update_fsm(fsm_t *fsm);

#define READY_TIMEOUT 1000 // 10s

#define IDLE_DEEPSLEEP_TIMEOUT 6000 // 60s

#define SHORT_BUTTON_PRESS 10 // 0.1s
#define LONG_BUTTON_PRESS 300 // 3s
#define TIMEOUT_BETWEEN_BUTTON_PRESS 300 // 3s

#define LIFTOFF_SAMPLES 20 // 0.2s

#define APOGEE_SAMPLES 50 // 0.5s
#define MINIMUM_ASCENT_TIME 300 // 3s

#define PARACHUTE_OPENING_TIME 200 // 2s
