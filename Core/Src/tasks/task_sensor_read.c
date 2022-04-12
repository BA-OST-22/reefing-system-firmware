/*
 * task_sensor_read.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */

#include "cmsis_os.h"
#include "task_sensor_read.h"
#include "config/globals.h"
#include "config/config.h"
#include "util/log.h"

#include "target/target.h"

#include "sensors/lsm6dsr.h"

LSM6DSR IMU = {
	.spi_handle = &hspi1,
	.accel_odr = LSM6DSR_XL_ODR_104Hz,
	.accel_range = LSM6DSR_16g,
	.gyro_odr = LSM6DSR_GY_ODR_OFF,
	.gyro_range = LSM6DSR_2000dps,
};



void task_sensor_read(void *argument) {

	uint32_t tick_count = osKernelGetTickCount();
	uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;

	lsm6dsr_init(&IMU);

	/* Set continuous conversion mode */
	lsm6dsr_enable(&IMU);

	while (1) {

		sensor_mode_e mode = osEventFlagsWait(sensor_mode_id, 0xFF, osFlagsWaitAny, 0);
		if(mode == SENSOR_MODE_CONTINUOUS){
			/* Set continuous conversion mode */
			lsm6dsr_enable(&IMU);
			lsm6dsr_wakeup_disable(&IMU);
		} else if (mode == SENSOR_MODE_WAKEUP){
			lsm6dsr_wakeup_enable(&IMU, global_config.config.liftoff_acc_threshold);
		} else if (mode == SENSOR_MODE_SHUTDOWN){
			lsm6dsr_shutdown(&IMU);
			osThreadTerminate(osThreadGetId());
		}


		float acceleration[3];
		lsm6dsr_get_accel(&IMU, acceleration);

		tick_count += tick_update;
		osDelayUntil(tick_count);
	}
}

