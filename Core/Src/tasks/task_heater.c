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

#include "task_heater.h"
#include "cmsis_os.h"
#include "config/globals.h"
#include "drivers/dcdc.h"
#include "sensors/max6675.h"
#include "target/target.h"
#include "util/log.h"

#define SAMPLING_FREQ_HEATER 10 // Hz

#define PREHEAT_VOLTAGE 5.2f
#define HEAT_VOLTAGE 12.2f

void task_heater(void *argument) {
  log_debug("Task Heater started");

  dcdc_disable();

  uint32_t tick_count = osKernelGetTickCount();
  uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ_HEATER;
  while (1) {

    heater_mode_e mode =
        osEventFlagsWait(heater_mode_id, 0xFF, osFlagsWaitAny, 0);

    if (mode == HEATER_MODE_IDLE) {
      dcdc_disable();
      HAL_GPIO_WritePin(CUT_EN_GPIO_Port, CUT_EN_Pin, GPIO_PIN_RESET);
    } else if (mode == HEATER_MODE_PREHEAT) {
      dcdc_set_voltage(PREHEAT_VOLTAGE);
      dcdc_enable();
      osDelay(100);
      HAL_GPIO_WritePin(CUT_EN_GPIO_Port, CUT_EN_Pin, GPIO_PIN_SET);
    } else if (mode == HEATER_MODE_HEAT) {
      dcdc_set_voltage(HEAT_VOLTAGE);
      dcdc_enable();
      osDelay(100);
      HAL_GPIO_WritePin(CUT_EN_GPIO_Port, CUT_EN_Pin, GPIO_PIN_SET);
    } else if (mode == HEATER_MODE_SHUTDOWN) {
      dcdc_disable();
      HAL_GPIO_WritePin(CUT_EN_GPIO_Port, CUT_EN_Pin, GPIO_PIN_RESET);
      log_debug("Task Heater stopped");
      osThreadTerminate(osThreadGetId());
    }

    float temperature = 0;
    thermocouple_status_e status;
    status = get_temperature(&temperature);

    if (status != TMP_OK) {
      log_error("No thermocouple connected!");
    } else {
      // log_info("Current thermocouple temperature: %ld",
      // (int32_t)temperature);
    }

    tick_count += tick_update;
    osDelayUntil(tick_count);
  }
}
