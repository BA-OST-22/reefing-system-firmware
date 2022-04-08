/*
 * task_sensor_read.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */

#include "cmsis_os.h"
#include "task_sensor_read.h"
#include "config/globals.h"
#include "util/log.h"

#include "target/target.h"

#include "sensors/lsm6dsr_reg.h"

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);

void task_sensor_read(void *argument) {

	static stmdev_ctx_t dev_ctx;

	uint32_t tick_count = osKernelGetTickCount();
	uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;

	dev_ctx.write_reg = platform_write;
	dev_ctx.read_reg = platform_read;
	dev_ctx.handle = &hspi1;

	uint8_t whoamI;
	lsm6dsr_pin_int1_route_t int1_route;

	lsm6dsr_device_id_get(&dev_ctx, &whoamI);

	if (whoamI != LSM6DSR_ID)
	  while (1);

	lsm6dsr_reset_set(&dev_ctx, PROPERTY_ENABLE);
	HAL_Delay(10);

	/* Disable I3C interface */
	lsm6dsr_i3c_disable_set(&dev_ctx, LSM6DSR_I3C_DISABLE);

	/* Enable Block Data Update */
	lsm6dsr_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
	/* Set Output Data Rate */
	lsm6dsr_xl_data_rate_set(&dev_ctx, LSM6DSR_XL_ODR_12Hz5);
	lsm6dsr_gy_data_rate_set(&dev_ctx, LSM6DSR_GY_ODR_12Hz5);
	/* Set full scale */
	lsm6dsr_xl_full_scale_set(&dev_ctx, LSM6DSR_16g);
	lsm6dsr_gy_full_scale_set(&dev_ctx, LSM6DSR_2000dps);
	/* Configure filtering chain(No aux interface)
	* Accelerometer - LPF1 + LPF2 path
	*/
	//lsm6dsr_xl_hp_path_on_out_set(&dev_ctx, LSM6DSR_LP_ODR_DIV_100);
	//lsm6dsr_xl_filter_lp2_set(&dev_ctx, PROPERTY_ENABLE);

	/* Apply high-pass digital filter on Wake-Up function */
	lsm6dsr_xl_hp_path_internal_set(&dev_ctx, LSM6DSR_USE_SLOPE);
	/* Set Wake-Up threshold: 1 LSb corresponds to FS_XL/2^6 */
	lsm6dsr_wkup_threshold_set(&dev_ctx, 2);

	/* interrupt generation on Wake-Up INT1 pin */
	lsm6dsr_pin_int1_route_get(&dev_ctx, &int1_route);
	int1_route.md1_cfg.int1_wu = PROPERTY_ENABLE;
	lsm6dsr_pin_int1_route_set(&dev_ctx, &int1_route);

	while (1) {


		int16_t data_raw_acceleration[3];
		float acceleration_mg[3];
		int16_t data_raw_angular_rate[3];

		lsm6dsr_acceleration_raw_get(&dev_ctx, data_raw_acceleration);
		acceleration_mg[0] = lsm6dsr_from_fs16g_to_mg(data_raw_acceleration[0]);
		acceleration_mg[1] = lsm6dsr_from_fs16g_to_mg(data_raw_acceleration[1]);
		acceleration_mg[2] =  lsm6dsr_from_fs16g_to_mg(data_raw_acceleration[2]);



		tick_count += tick_update;
		osDelayUntil(tick_count);
	}
}

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
  HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(handle, &reg, 1, 2);
  HAL_SPI_Transmit(handle, (uint8_t*) bufp, len, 2);
  HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);
  return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
  reg |= 0x80;
  HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(handle, &reg, 1, 1000);
  HAL_SPI_Receive(handle, bufp, len, 1000);
  HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);
  return 0;
}
