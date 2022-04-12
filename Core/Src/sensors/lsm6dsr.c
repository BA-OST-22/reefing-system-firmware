/*
 * lsm6dsr.c
 *
 *  Created on: 11 Apr 2022
 *      Author: Luca
 */

#include "lsm6dsr.h"

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);

static stmdev_ctx_t ctx;

bool lsm6dsr_init(LSM6DSR* dev){
	dev->dev_ctx = &ctx;
	dev->dev_ctx->write_reg = platform_write;
	dev->dev_ctx->read_reg = platform_read;
	dev->dev_ctx->handle = dev->spi_handle;

	uint8_t whoamI;

	lsm6dsr_device_id_get(dev->dev_ctx, &whoamI);

	if (whoamI != LSM6DSR_ID)
	  return false;

	lsm6dsr_reset_set(dev->dev_ctx, PROPERTY_ENABLE);
	osDelay(10);

	/* Disable I3C interface */
	lsm6dsr_i3c_disable_set(dev->dev_ctx, LSM6DSR_I3C_DISABLE);

	/* Set full scale */
	lsm6dsr_xl_full_scale_set(dev->dev_ctx, dev->accel_range);
	lsm6dsr_gy_full_scale_set(dev->dev_ctx, dev->gyro_range);

	return true;
}

void lsm6dsr_shutdown(LSM6DSR* dev){
	/* Set Output Data Rate to 0 */
	lsm6dsr_xl_data_rate_set(dev->dev_ctx, LSM6DSR_XL_ODR_OFF);
	lsm6dsr_gy_data_rate_set(dev->dev_ctx, LSM6DSR_GY_ODR_OFF);
}

void lsm6dsr_enable(LSM6DSR* dev){
	/* Set Output Data Rate */
	lsm6dsr_xl_data_rate_set(dev->dev_ctx, dev->accel_odr);
	lsm6dsr_gy_data_rate_set(dev->dev_ctx, dev->gyro_odr);
}

// Threshold in m/s^2
void lsm6dsr_wakeup_enable(LSM6DSR* dev, uint32_t threshold_ms2){

	lsm6dsr_xl_data_rate_set(dev->dev_ctx, dev->accel_odr);

	lsm6dsr_xl_full_scale_set(dev->dev_ctx, dev->accel_range);
	/* Apply high-pass digital filter on Wake-Up function */
	lsm6dsr_xl_hp_path_internal_set(dev->dev_ctx, LSM6DSR_USE_SLOPE);
	/* Set Wake-Up threshold: 1 LSb corresponds to FS_XL/2^6 */
	uint8_t threshold = (uint8_t)((float)threshold_ms2 / ((16.0f*9.81f)/64.0f));
	lsm6dsr_wkup_threshold_set(dev->dev_ctx, threshold);

	lsm6dsr_pin_int1_route_t int1_route;
	lsm6dsr_pin_int1_route_get(dev->dev_ctx, &int1_route);
	int1_route.md1_cfg.int1_wu = PROPERTY_ENABLE;
	lsm6dsr_pin_int1_route_set(dev->dev_ctx, &int1_route);
}

void lsm6dsr_wakeup_disable(LSM6DSR* dev){
	lsm6dsr_pin_int1_route_t int1_route;
	lsm6dsr_pin_int1_route_get(dev->dev_ctx, &int1_route);
	int1_route.md1_cfg.int1_wu = PROPERTY_DISABLE;
	lsm6dsr_pin_int1_route_set(dev->dev_ctx, &int1_route);
}


float get_accel_conversion(LSM6DSR* dev){
	switch(dev->accel_range){
		case LSM6DSR_2g:
			return 0.00059841f;
			break;
		case LSM6DSR_4g:
			return 0.00119682f;
			break;
		case LSM6DSR_8g:
			return 0.00239364f;
			break;
		case LSM6DSR_16g:
			return 0.00478728f;
			break;
		default:
			return 0;
			break;
	}
}

void lsm6dsr_get_accel(LSM6DSR* dev, float* acceleration){
	int16_t a[3];
	lsm6dsr_acceleration_raw_get(dev->dev_ctx, a);
	for(int i = 0; i < 3; i++){
		acceleration[i] = (float)a[i] * get_accel_conversion(dev);
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
