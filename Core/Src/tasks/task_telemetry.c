/*
 * task_telemetry.c
 *
 *  Created on: 12.04.2022
 *      Author: luca.jost
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
