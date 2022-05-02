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

#pragma once

#include "target/target.h"

static inline void dcdc_enable() {
  /* Never turn on dcdc converter when usb is connected */
  if (HAL_GPIO_ReadPin(USB_DET_GPIO_Port, USB_DET_Pin) == 0) {
    HAL_GPIO_WritePin(DCDC_EN_GPIO_Port, DCDC_EN_Pin, GPIO_PIN_SET);
  }
}

static inline void dcdc_disable() {
  HAL_GPIO_WritePin(DCDC_EN_GPIO_Port, DCDC_EN_Pin, GPIO_PIN_RESET);
}

void dcdc_set_voltage(float voltage);
float dcdc_get_voltage();
