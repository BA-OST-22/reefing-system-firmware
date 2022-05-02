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

#include "task_buzzer.h"
#include "cmsis_os.h"
#include "config/config.h"
#include "config/globals.h"
#include "util/log.h"

#include "drivers/buzzer.h"

#define BUZZER_COMMAND_MAX_LENGTH 9

#define BUZZER_SHORT_BEEP 40
#define BUZZER_LONG_BEEP 100
#define BUZZER_SHORT_PAUSE 100
#define BUZZER_LONG_PAUSE 1000

static BUZ_DEV BUZZER = {.timer = &htim4,
                         .channel = TIM_CHANNEL_1,
                         .arr = 4000,
                         .start = 0,
                         .started = 0,
                         .volume = 100};

static const uint32_t pitch_lookup[8] = {
    2217, // C# 	A
    2349, // D 	B
    2489, // D# 	C
    2637, // E 	D
    2793, // F 	E
    2959, // F# 	F
    3135, // G 	G
    1200, // Error H
};

static const uint8_t nr_buz[7] = {0, 1, 1, 4, 3, 3, 2};

static const char beep_codes[8][BUZZER_COMMAND_MAX_LENGTH] = {
    " ",
    "g",    // ok
    "h",    // nok
    "DBFG", // bootup
    "ace",  // ready
    "eca",  // not ready
    "ff",   // recovery
    "aa",
};

void task_buzzer(void *argument) {
  log_debug("Task Buzzer started");

  beeps_e id;
  buzzer_set_freq(&BUZZER, 2500);

  while (1) {
    id = osEventFlagsWait(buzzer_event_id, 0xFF, osFlagsWaitAny, osWaitForever);
    osEventFlagsClear(buzzer_event_id, id);
    buzzer_set_volume(&BUZZER, global_config.config.buzzer_volume);
    uint32_t duration = 0;
    for (int i = 0; i < nr_buz[id]; i++) {
      char pitch = beep_codes[id][i];
      if (pitch >= 'A' && pitch <= 'H') {
        buzzer_set_freq(&BUZZER, pitch_lookup[pitch - 'A']);
        duration = BUZZER_LONG_BEEP;
      } else if (pitch >= 'a' && pitch <= 'h') {
        buzzer_set_freq(&BUZZER, pitch_lookup[pitch - 'a']);
        duration = BUZZER_SHORT_BEEP;
      }
      buzzer_start(&BUZZER);
      osDelay(duration);
      buzzer_stop(&BUZZER);
      osDelay(BUZZER_SHORT_PAUSE);
    }

    // Wait at least 1s before buzzing again
    osDelay(BUZZER_LONG_PAUSE);
  }
}
