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

#include "globals.h"

/* USB buffers and fifos */

fifo_t usb_input_fifo;
fifo_t usb_output_fifo;

uint8_t usb_fifo_out_buffer[USB_OUTPUT_BUFFER_SIZE];
uint8_t usb_fifo_in_buffer[USB_INPUT_BUFFER_SIZE];

bool global_usb_initialized_flag = false;

osEventFlagsId_t buzzer_event_id;
osEventFlagsId_t sensor_mode_id;
osEventFlagsId_t state_est_mode_id;
osEventFlagsId_t telemetry_mode_id;
osEventFlagsId_t heater_mode_id;
osEventFlagsId_t supervisor_mode_id;

baro_data_t global_baro_data;
imu_data_t global_imu_data;
bool global_light;

state_data_t global_state_data;

recorder_t recorder;

fsm_state_e global_flight_state = IDLE;
