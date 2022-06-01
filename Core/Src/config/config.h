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

#include <stdbool.h>
#include <stdint.h>

/* The system will reload the default config when the number changes */
/* Config version 1 / Minor 2 */
#define CONFIG_VERSION 103

typedef struct {
  /* Needs to be in first position */
  uint32_t config_version;
  uint32_t main_altitude;
  uint32_t liftoff_acc_threshold;
  uint32_t timer_duration;
  uint8_t enable_telemetry;
  uint8_t enable_preheat;
  uint8_t link_phrase[10];
  uint8_t buzzer_volume;
  uint8_t log_every_n;
  uint8_t burn_duration;
  uint8_t apogee_delay;
} config_t;

typedef union {
  config_t config;
  uint8_t config_array[sizeof(config_t)];
} config_u;

extern config_u global_config;

/** cats config initialization **/
void config_init();
void config_defaults();

/** persistence functions **/
void config_load();
bool config_save();
