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

#include "max6675.h"

#define MAX_SPI_HANDLE hspi2

/**
 * @brief Get the temperature from the MAX6675 sensor
 *
 * @param temperature: Pointer to the temperature variable
 * @return thermocouple_status_e: Status of the thermocouple
 */
thermocouple_status_e get_temperature(float *temperature) {
  uint8_t tmp[2];
  HAL_GPIO_WritePin(TC_CS_GPIO_Port, TC_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Receive(&MAX_SPI_HANDLE, tmp, 2, 2);
  HAL_GPIO_WritePin(TC_CS_GPIO_Port, TC_CS_Pin, GPIO_PIN_SET);
  if (tmp[1] & 0x04) {
    return TMP_OPEN;
  } else {
    *temperature = (((uint16_t)(tmp[0]) << 5) + ((tmp[1] & 0xF8) >> 3)) / 4.0f;
    return TMP_OK;
  }
}
