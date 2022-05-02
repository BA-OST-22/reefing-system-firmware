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

#include "task_sensor_read.h"
#include "cmsis_os.h"
#include "config/config.h"
#include "config/globals.h"
#include "drivers/adc.h"
#include "drivers/pm.h"
#include "util/log.h"

#include "target/target.h"

#include "sensors/light.h"
#include "sensors/lsm6dsr.h"
#include "sensors/ms5607.h"

LSM6DSR IMU = {
    .spi_handle = &hspi1,
    .accel_odr = LSM6DSR_XL_ODR_104Hz,
    .accel_range = LSM6DSR_16g,
    .gyro_odr = LSM6DSR_GY_ODR_OFF,
    .gyro_range = LSM6DSR_2000dps,
};

MS5607 BARO = {
    .spi_handle = &hspi1,
    .cs_port = BARO_CS_GPIO_Port,
    .cs_pin = BARO_CS_Pin,
    .osr = MS5607_OSR_1024,
};

LIGHT_SENSOR LIGHT = {
    .source = ADC_SOURCE_LIGHT,
};

void task_sensor_read(void *argument) {

  log_debug("Task Sensor Read started");

  /* Enable CUT power rail as light sensor is on there */
  pm_cut_on();

  lsm6dsr_init(&IMU);
  ms5607_init(&BARO);
  light_init(&LIGHT);

  /* Set continuous conversion mode */
  lsm6dsr_enable(&IMU);

  uint8_t stage = 0;

  uint32_t tick_count = osKernelGetTickCount();
  uint32_t tick_update = osKernelGetTickFreq() / (2 * SAMPLING_FREQ);
  while (1) {

    sensor_mode_e mode =
        osEventFlagsWait(sensor_mode_id, 0xFF, osFlagsWaitAny, 0);
    if (mode == SENSOR_MODE_CONTINUOUS) {
      /* Set continuous conversion mode */
      lsm6dsr_enable(&IMU);
      lsm6dsr_wakeup_disable(&IMU);
    } else if (mode == SENSOR_MODE_WAKEUP) {
      /* Put sensor in Wake-up mode */
      lsm6dsr_wakeup_enable(&IMU, 5);
    } else if (mode == SENSOR_MODE_SHUTDOWN) {
      /* Disable the sensor and terminate the task */
      log_debug("Task Sensor Read stopped");
      lsm6dsr_shutdown(&IMU);
      pm_cut_off();
      osThreadTerminate(osThreadGetId());
    }

    /* Stage 1: Read all sensor data and save it in global register */
    if (stage == 1) {
      /* Readout pressure data */
      ms5607_read_raw(&BARO);
      /* Store pressure and temperature data*/
      ms5607_get_temp_pres(&BARO, &global_baro_data.temperature,
                           &global_baro_data.pressure);
      /* Prepare temperature data */
      ms5607_prepare_temp(&BARO);

      /* Readout acceleration */
      lsm6dsr_get_accel(&IMU, global_imu_data.acceleration);

      /* Readout light sensor */
      // log_info("Light sensor %ld",light_get(&LIGHT));

      stage = 0;
    }
    /* Stage 0: Readout the Baro temperature and prepare pressure */
    else {
      /* Readout temperature data */
      ms5607_read_raw(&BARO);
      /* Prepare pressure data */
      ms5607_prepare_pres(&BARO);
      stage = 1;
    }

    tick_count += tick_update;
    osDelayUntil(tick_count);
  }
}
