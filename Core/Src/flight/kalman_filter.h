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

typedef struct {
  float altitude_agl;
  float Ad[2 * 2];
  float Ad_T[2 * 2];
  float GdQGd_T[2 * 2];
  float H[1 * 2];
  float H_T[2 * 1];
  float K[2 * 1];
  float x_bar[2 * 1];
  float x_hat[2 * 1];
  float P_bar[2 * 2];
  float P_hat[2 * 2];
  float R;
  float ts;
} kalman_filter_t;

void kalman_init(kalman_filter_t *filter);
void kalman_predict(kalman_filter_t *filter);
void kalman_update(kalman_filter_t *filter);
