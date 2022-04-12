#pragma once

#include "lsm6dsr_reg.h"
#include "target/target.h"
#include "cmsis_os.h"
#include <stdbool.h>



typedef struct lsm6dsr_dev {
  // Hardware Configuration
  SPI_HandleTypeDef *const spi_handle;

  // Sensor Configuration
  stmdev_ctx_t* dev_ctx;

  lsm6dsr_fs_xl_t accel_range;
  lsm6dsr_odr_xl_t accel_odr;

  lsm6dsr_fs_g_t gyro_range;
  lsm6dsr_odr_g_t gyro_odr;
} LSM6DSR;


bool lsm6dsr_init(LSM6DSR* dev);
void lsm6dsr_shutdown(LSM6DSR* dev);
void lsm6dsr_enable(LSM6DSR* dev);

// Threshold in m/s^2
void lsm6dsr_wakeup_enable(LSM6DSR* dev, uint32_t threshold_ms2);
void lsm6dsr_wakeup_disable(LSM6DSR* dev);

void lsm6dsr_get_accel(LSM6DSR* dev, float* acceleration);
