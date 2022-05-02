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

static inline void pm_cut_on() {
  HAL_GPIO_WritePin(P_EN_CUT_GPIO_Port, P_EN_CUT_Pin, GPIO_PIN_SET);
}

static inline void pm_cut_off() {
  HAL_GPIO_WritePin(P_EN_CUT_GPIO_Port, P_EN_CUT_Pin, GPIO_PIN_RESET);
}

static inline void pm_radio_on() {
  HAL_GPIO_WritePin(P_EN_RADIO_GPIO_Port, P_EN_RADIO_Pin, GPIO_PIN_SET);
}

static inline void pm_radio_off() {
  HAL_GPIO_WritePin(P_EN_RADIO_GPIO_Port, P_EN_RADIO_Pin, GPIO_PIN_RESET);
}
