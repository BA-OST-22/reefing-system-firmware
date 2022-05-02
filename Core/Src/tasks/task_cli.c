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

#include "task_cli.h"
#include "cli/cli.h"
#include "cmsis_os.h"
#include "config/globals.h"
#include "util/log.h"

void task_cli(void *argument) {
  log_debug("Task CLI started");

  cli_enter(&usb_input_fifo, &usb_output_fifo);

  uint32_t tick_count = osKernelGetTickCount();
  uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;
  while (1) {
    cli_process();

    tick_count += tick_update;
    osDelayUntil(tick_count);
  }
}
