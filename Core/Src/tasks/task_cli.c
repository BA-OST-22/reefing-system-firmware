/*
 * task_cli.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
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
