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

#include "sensors/ms5607.h"
#include <stdbool.h>

// Commands
#define COMMAND_RESET 0x1E
#define COMMAND_CONVERT_D1_BASE 0x40
#define COMMAND_CONVERT_D2_BASE 0x50
#define COMMAND_ADC_READ 0x00
#define COMMAND_PROM_READ_BASE 0xA0

// Conversion time
#define BARO_CONVERSION_TIME_OSR_BASE 0.6f

/** Private Function Declarations **/

static uint32_t get_conversion_ticks(MS5607 *dev);
// Read bytes
static void ms_read_bytes(MS5607 *dev, uint8_t command, uint8_t *pData,
                          uint16_t size);
// Write command
static void ms_write_command(MS5607 *dev, uint8_t command);
static void read_calibration(MS5607 *dev);

uint16_t uint8_to_uint16(uint8_t src_high, uint8_t src_low) {
  return (src_high << 8 | src_low);
}

/** Exported Function Definitions **/

/**
 * @brief Initialize the barometer
 *
 * @param dev: Pointer to MS5607 device
 * @return None
 */
void ms5607_init(MS5607 *dev) {
  uint32_t reset_time;
  reset_time = 3 * osKernelGetTickFreq() / 1000;
  // General Procedure:
  //  1. reset chip
  //  2. Read out calibration

  // Reset chip
  ms_write_command(dev, COMMAND_RESET);
  osDelay(reset_time);

  // Read calibration
  read_calibration(dev);
  osDelay(1);
}

void ms5607_deinit(MS5607 *dev) { ms_write_command(dev, COMMAND_RESET); }

void ms5607_prepare_temp(MS5607 *dev) {
  uint8_t command;
  dev->data = MS5607_TEMPERATURE;
  command = COMMAND_CONVERT_D2_BASE + (dev->osr * 2);
  ms_write_command(dev, command);
}

void ms5607_prepare_pres(MS5607 *dev) {
  uint8_t command;
  dev->data = MS5607_PRESSURE;
  command = COMMAND_CONVERT_D1_BASE + (dev->osr * 2);
  ms_write_command(dev, command);
}

/**
 *@brief Read the temperature and pressure from the MS5607 sensor
 *
 * @param dev: Pointer to MS5607 device
 * @param temperature: Pointer to temperature variable
 * @param pressure: Pointer to pressure variable
 * @return True on success
 */
bool ms5607_get_temp_pres(MS5607 *dev, int32_t *temperature,
                          int32_t *pressure) {
  int64_t OFF, SENS;
  int64_t dT;
  int32_t temp, pres;

  temp = (dev->raw_temp[0] << 16) + (dev->raw_temp[1] << 8) + dev->raw_temp[2];
  dT = temp - ((int32_t)dev->coefficients[4] << 8);
  /* Temperature in 2000  = 20.00° C */
  *temperature = (int32_t)2000 + (dT * dev->coefficients[5] >> 23);

  pres = (dev->raw_pres[0] << 16) + (dev->raw_pres[1] << 8) + dev->raw_pres[2];
  OFF = ((int64_t)dev->coefficients[1] << 17) +
        ((dev->coefficients[3] * dT) >> 6);
  SENS = ((int64_t)dev->coefficients[0] << 16) +
         ((dev->coefficients[2] * dT) >> 7);
  /* Pressure in 110002 = 1100.02 mbar */
  *pressure = (int32_t)((((pres * SENS) >> 21) - OFF) >> 15);
  return true;
}

/**
 * @brief Get the air pressure from the MS5607 sensor
 *
 * @param dev: Pointer to MS5607 device
 * @param pressure: Pointer to pressure variable
 * @return True on success
 */
bool ms5607_get_pres(MS5607 *dev, int32_t *pressure) {
  int64_t OFF, SENS;
  int32_t pres, temp;
  int64_t dT;

  temp = (dev->raw_temp[0] << 16) + (dev->raw_temp[1] << 8) + dev->raw_temp[2];
  dT = temp - ((int32_t)dev->coefficients[4] << 8);

  if (dev->data == MS5607_PRESSURE) {
    ms5607_read_raw(dev);
  } else
    return false;

  pres = (dev->raw_pres[0] << 16) + (dev->raw_pres[1] << 8) + dev->raw_pres[2];
  OFF = ((int64_t)dev->coefficients[1] << 17) +
        ((dev->coefficients[3] * dT) >> 6);
  SENS = ((int64_t)dev->coefficients[0] << 16) +
         ((dev->coefficients[2] * dT) >> 7);
  /* Pressure in 110002 = 1100.02 mbar */
  *pressure = (int32_t)((((pres * SENS) >> 21) - OFF) >> 15);
  return true;
}

/** Private Function Definitions **/

/**
 * @brief Get the conversion ticks for the current OSR
 *
 * @param dev: Pointer to MS5607 device
 * @return uint32_t: Conversion ticks in ms
 */
static uint32_t get_conversion_ticks(MS5607 *dev) {
  uint32_t time;
  time = (BARO_CONVERSION_TIME_OSR_BASE * ((float)dev->osr + 1) *
          osKernelGetTickFreq()) /
         1000;
  if (time < 1)
    time = 1;
  return time;
}

/**
 * @brief Read data over SPI from the MS5607 sensor
 *
 * @param dev: Pointer to MS5607 device
 * @param command: Command to send
 * @param pData: Pointer to data buffer
 * @param size: Size of data buffer
 */
static void ms_read_bytes(MS5607 *dev, uint8_t command, uint8_t *pData,
                          uint16_t size) {
  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(dev->spi_handle, &command, 1, 1);
  HAL_SPI_Receive(dev->spi_handle, pData, size, 1);
  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}

/**
 * @brief Write data over SPI to the MS5607 sensor
 *
 * @param dev: Pointer to MS5607 device
 * @param command: Command to send
 * @param pData: Pointer to data buffer
 * @param size: Size of data buffer
 */
static void ms_write_command(MS5607 *dev, uint8_t command) {
  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(dev->spi_handle, &command, 1, 1);
  HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}

/**
 * @brief Read calibration data from the MS5607 sensor
 *
 * @param dev: Pointer to MS5607 device
 */
static void read_calibration(MS5607 *dev) {
  for (int i = 0; i < 6; i++) {
    uint8_t rec[2] = {0};
    ms_read_bytes(dev, COMMAND_PROM_READ_BASE + (2 * (i + 1)), rec, 2);
    dev->coefficients[i] = uint8_to_uint16(rec[0], rec[1]);
  }
}

/**
 * @brief Read raw data from the MS5607 sensor
 *
 * @param dev: Pointer to MS5607 device
 */
bool ms5607_read_raw(MS5607 *dev) {
  if (dev->data == MS5607_PRESSURE)
    ms_read_bytes(dev, COMMAND_ADC_READ, dev->raw_pres, 3);
  else if (dev->data == MS5607_TEMPERATURE)
    ms_read_bytes(dev, COMMAND_ADC_READ, dev->raw_temp, 3);
  else
    return false;
  dev->data = MS5607_PRESSURE;
  return true;
}
