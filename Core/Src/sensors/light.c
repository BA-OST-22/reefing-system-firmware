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

#include "sensors/light.h"

/**
 * @brief  Initialize the light sensor
 *
 * @param  dev: light sensor structure
 * @retval None
 */
void light_init(LIGHT_SENSOR *dev) { dev->dark = -1; }

/**
 * @brief  Set the dark value, call this function when the sensor is obstructed,
 * the current brightness is stored as a reference for
 * the deployment
 *
 * @param  dev: light sensor structure
 * @retval None
 */
void light_set_dark(LIGHT_SENSOR *dev) { dev->dark = adc_get(dev->source); }

/**
 * @brief Set the threshold for the light sensor to detect light
 *
 * @param dev: light sensor structure
 * @param threshold: threshold value
 */
void light_set_threshold(LIGHT_SENSOR *dev, uint32_t threshold) {
  dev->threshold = threshold;
}

/**
 * @brief Get the current state of the light sensor
 *
 * @param dev: light sensor structure
 * @return light_state_e: light state
 */
light_state_e light_get_state(LIGHT_SENSOR *dev) {
  if (dev->dark == -1)
    return LIGHT_UNDEFINED;
  int32_t light = adc_get(dev->source);
  if ((light - dev->dark) > dev->threshold) {
    return LIGHT_BRIGHT;
  } else {
    return LIGHT_DARK;
  }
}

/**
 * @brief Get the current brightness of the light sensor
 *
 * @param dev: light sensor structure
 * @return uint32_t: brightness value
 */
uint32_t light_get(LIGHT_SENSOR *dev) { return adc_get(dev->source); }
