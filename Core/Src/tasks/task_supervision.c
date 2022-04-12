/*
 * task_supervision.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */

#include "cmsis_os.h"
#include "task_supervision.h"
#include "config/globals.h"
#include "util/log.h"
#include "drivers/dcdc.h"
#include "usb_device.h"
#include "init/init.h"

void task_supervision(void *argument) {

	bool cli_stared = false;

	uint32_t tick_count = osKernelGetTickCount();
	uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;

	while (1) {


		/* Check the USB detection pin */
		if(HAL_GPIO_ReadPin(USB_DET_GPIO_Port, USB_DET_Pin)){
			/* Make sure the DCDC converter is never turned on */
			dcdc_disable();

			/* If usb is not initialized yet, do so */
			if(global_usb_initialized_flag == false){
				global_usb_initialized_flag = true;
				MX_USB_DEVICE_Init();
				log_enable();
			}
		}

		/* Start cli when something was received from the usb interface */
		if(fifo_get_length(&usb_input_fifo) && cli_stared == false){
			cli_stared = true;
			init_cli();
		}

		tick_count += tick_update;
		osDelayUntil(tick_count);
	}
}
