/*
 * globals.h
 *
 *  Created on: Apr 5, 2022
 *      Author: Luca
 */

#pragma once

#include "cmsis_os.h"
#include "recorder/recorder.h"
#include "util/fifo.h"

/* Sampling rate for all time dependent operations */
#define SAMPLING_FREQ 100 // Hz

/* USB fifos */

#define USB_OUTPUT_BUFFER_SIZE 512
#define USB_INPUT_BUFFER_SIZE 512

extern fifo_t usb_input_fifo;
extern fifo_t usb_output_fifo;

extern uint8_t usb_fifo_out_buffer[USB_OUTPUT_BUFFER_SIZE];
extern uint8_t usb_fifo_in_buffer[USB_INPUT_BUFFER_SIZE];

extern bool global_usb_initialized_flag;

typedef enum {
  NO_BEEP = 0,
  BEEP_OK,
  BEEP_NOK,
  BEEP_BOOTUP,
  BEEP_READY,
  BEEP_NOT_READY,
  BEEP_RECOVERY,
} beeps_e;

extern osEventFlagsId_t buzzer_event_id;

typedef enum {
  SENSOR_MODE_CONTINUOUS = 1,
  SENSOR_MODE_WAKEUP,
  SENSOR_MODE_SHUTDOWN,
} sensor_mode_e;

extern osEventFlagsId_t sensor_mode_id;

typedef enum {
  STATE_EST_MODE_ASCENT = 1,
  STATE_EST_MODE_DECENT,
  STATE_EST_MODE_SHUTDOWN,
} state_est_mode_e;

extern osEventFlagsId_t state_est_mode_id;

typedef enum {
  TELEMETRY_MODE_SHUTDOWN = 1,
} telemetry_mode_e;

extern osEventFlagsId_t telemetry_mode_id;

typedef enum {
  HEATER_MODE_IDLE = 1,
  HEATER_MODE_PREHEAT,
  HEATER_MODE_HEAT,
  HEATER_MODE_SHUTDOWN,
} heater_mode_e;

extern osEventFlagsId_t heater_mode_id;

typedef enum {
  SUPERVISOR_MODE_SHUTDOWN = 1,
} supervisor_mode_e;
extern osEventFlagsId_t supervisor_mode_id;

typedef struct {
  int32_t pressure;
  int32_t temperature;
} baro_data_t;

typedef struct {
  float acceleration[3];
} imu_data_t;

extern baro_data_t global_baro_data;
extern imu_data_t global_imu_data;
extern bool global_light;

typedef struct {
  float altitude_agl;
  float velocity;
} state_data_t;

extern state_data_t global_state_data;

typedef enum {
  INVALID = 0,
  IDLE = 1,
  DEEP_SLEEP,
  READY,
  ASCENT,
  DESCENT,
  DEPLOYMENT,
  RECOVERY,
} fsm_state_e;

extern fsm_state_e global_flight_state;

#define RECORDER_SIZE (1024 * 64)
extern recorder_t recorder;
