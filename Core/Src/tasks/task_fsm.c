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

#include "task_fsm.h"
#include "cmsis_os.h"

#include "config/config.h"
#include "config/globals.h"

#include "flight/fsm.h"
#include "util/log.h"

#include "target/target.h"

void task_fsm(void *argument) {

  log_debug("Task FSM started");

  fsm_t fsm;

  fsm.flight_state = IDLE;
  fsm.memory[0] = 0;
  fsm.memory[1] = 0;
  fsm.memory[2] = 0;
  fsm.memory[3] = 0;

  osEventFlagsSet(buzzer_event_id, BEEP_BOOTUP);

  uint32_t tick_count = osKernelGetTickCount();
  uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;
  while (1) {
    /* When we switch to another state, wait for the button to be released */
    if (fsm.state_changed && (fsm.flight_state < ASCENT)) {
      if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin)) {
        fsm.state_changed = false;
      }
    } else {
      update_fsm(&fsm);
    }

    global_flight_state = fsm.flight_state;

    tick_count += tick_update;
    osDelayUntil(tick_count);
  }
}
