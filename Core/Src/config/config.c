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

#include "config/eeprom.h"
#include <config/config.h>
#include <string.h>

const config_u DEFAULT_CONFIG = {
    .config.config_version = CONFIG_VERSION,
    .config.main_altitude = 150,
    .config.liftoff_acc_threshold = 35,
    .config.timer_duration = 0,
    .config.enable_telemetry = 0,
    .config.enable_preheat = 0,
    .config.buzzer_volume = 20,
    .config.burn_duration = 25,
    .config.log_every_n = 2,
    .config.apogee_delay = 2,
};

config_u global_config = {};

void config_init() {}

void config_defaults() {
  memcpy(&global_config, &DEFAULT_CONFIG, sizeof(global_config));
}

/** persistence functions **/
void config_load() { ee_read(0, sizeof(config_t), global_config.config_array); }

bool config_save() {
  bool status = false;
  status = ee_format();
  if (status == false)
    return status;
  return ee_write(0, sizeof(config_t), global_config.config_array);
}
