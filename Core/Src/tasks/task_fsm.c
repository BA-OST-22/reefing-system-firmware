/*
 * task_fsm.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */


#include "cmsis_os.h"
#include "task_fsm.h"

#include "config/globals.h"
#include "config/config.h"

#include "util/log.h"
#include "cli/cli.h"

void task_fsm(void *argument) {

	uint32_t tick_count = osKernelGetTickCount();
	uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;

	osDelay(2000);

	config_init();
	config_load();

	if(global_config.config.config_version != CONFIG_VERSION){
		log_warn("Config versions mismatch");
		log_info("Resetting to defaults..");
		config_defaults();
		bool status = config_save();
		if(status){
			log_info("Reset successful!");
		} else {
			log_error("Error resetting config!");
		}
	} else {
		log_info("Config loading successful!");
	}

	osEventFlagsSet(buzzer_event_id, BEEP_BOOTUP);

	//ee_write(12, 10, data_write);
	//ee_writeToRam(uint32_t startVirtualAddress, uint32_t len, uint8_t* data); //  only use when _EE_USE_RAM_BYTE is enabled
	log_disable();
	cli_enter(&usb_input_fifo, &usb_output_fifo);

	while (1) {

		cli_process();
		tick_count += tick_update;
		osDelayUntil(tick_count);


	}
}
