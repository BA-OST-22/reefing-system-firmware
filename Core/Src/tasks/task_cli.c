/*
 * task_cli.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */

#include "cmsis_os.h"
#include "task_cli.h"
#include "config/globals.h"
#include "util/log.h"
#include "cli/cli.h"


void task_cli(void *argument) {
	uint32_t tick_count = osKernelGetTickCount();
	uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;

	cli_enter(&usb_input_fifo, &usb_output_fifo);
	while (1) {
		cli_process();

		tick_count += tick_update;
		osDelayUntil(tick_count);
	}
}
