/*
 * task_heater.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */

#include "task_heater.h"
#include "cmsis_os.h"
#include "config/globals.h"
#include "drivers/dcdc.h"
#include "sensors/max6675.h"
#include "target/target.h"
#include "util/log.h"

#define SAMPLING_FREQ_HEATER 10 // Hz

#define PREHEAT_VOLTAGE 6.0f
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
