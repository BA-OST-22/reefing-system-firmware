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

#include "drivers/adc.h"

typedef enum { LIGHT_UNDEFINED = 1, LIGHT_DARK, LIGHT_BRIGHT } light_state_e;

typedef struct {
  int32_t dark;
  adc_source_t source;
  uint32_t threshold;
} LIGHT_SENSOR;

void light_init(LIGHT_SENSOR *dev);
void light_set_dark(LIGHT_SENSOR *dev);
light_state_e light_get_state(LIGHT_SENSOR *dev);
uint32_t light_get(LIGHT_SENSOR *dev);
