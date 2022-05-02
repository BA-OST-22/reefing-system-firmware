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

#include "sensors/battery.h"
#include "target/target.h"

/**
 * @brief  Get the battery voltage
 *
 * @param  battery: battery structure
 * @param  voltage: pointer to where the voltage will be stored
 * @retval battery_state_e: battery state
 */
battery_state_e battery_voltage(BATTERY_SENSOR *dev, float *voltage) {
  float volt = (float)adc_get(dev->source);

  *voltage = volt * dev->multiply + dev->add;

  if (HAL_GPIO_ReadPin(CHRG_GPIO_Port, CHRG_Pin)) {
    return BATTERY_CHARGE;
  } else {
    return BATTERY_DISCHARGE;
  }
}
