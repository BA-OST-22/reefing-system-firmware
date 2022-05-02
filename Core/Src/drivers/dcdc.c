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

#include "dcdc.h"
#include "cmsis_os.h"
#include "math.h"

#define DCDC_SPI_HANDLE hspi2

static float voltage = 0;

void dcdc_set_voltage(float volt) {
  uint8_t r_value;
  if (volt > 12.2f)
    volt = 12.2f;
  else if (volt < 5.2f)
    volt = 5.2f;

  voltage = volt;

  // Cubic least square fit
  // https://www.wolframalpha.com/input?i=cubic+fit+%7B12.2%2C0%7D%2C%7B11.48%2C10%7D%2C%7B8.83%2C64%7D%2C%7B7.04%2C128%7D%2C%7B6.43%2C160%7D%2C%7B5.21%2C255%7D
  r_value = -0.791728f * powf(voltage, 3) + 25.66f * powf(voltage, 2) -
            293.662f * voltage + 1199.67f;

  HAL_GPIO_WritePin(R_CS_GPIO_Port, R_CS_Pin, GPIO_PIN_RESET);
  osDelay(1);
  HAL_SPI_Transmit(&DCDC_SPI_HANDLE, &r_value, 1, 2);
  osDelay(1);
  HAL_GPIO_WritePin(R_CS_GPIO_Port, R_CS_Pin, GPIO_PIN_SET);
}

float dcdc_get_voltage() { return voltage; }
