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

#include "tasks/task_telemetry.h"
#include "cmsis_os.h"
#include "config/globals.h"
#include "drivers/pm.h"
#include "util/log.h"

#define SAMPLING_FREQ_RADIO 10

void task_telemetry(void *argument) {
  log_debug("Task Telemetry started");

  pm_radio_on();

  uint32_t tick_count = osKernelGetTickCount();
  uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ_RADIO;
  while (1) {

    telemetry_mode_e mode =
        osEventFlagsWait(telemetry_mode_id, 0xFF, osFlagsWaitAny, 0);

    if (mode == TELEMETRY_MODE_SHUTDOWN) {
      log_debug("Task Telemetry stopped");
      pm_radio_off();
      osThreadTerminate(osThreadGetId());
    }

    tick_count += tick_update;
    osDelayUntil(tick_count);
  }
}
