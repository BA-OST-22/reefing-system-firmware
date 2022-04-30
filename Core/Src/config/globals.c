/*
 * globals.c
 *
 *  Created on: Apr 5, 2022
 *      Author: Luca
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
