/*
 * task_state_est.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */

#include "task_state_est.h"
#include "cmsis_os.h"
#include "config/globals.h"
#include "flight/kalman_filter.h"
#include "util/log.h"
#include <math.h>

#define TEMPERATURE_0 15.0f

#define RATE_LIMIT_R 200.0f
#define RATE_LIMIT_F -30.0f
#define TIMESTEP 0.01f

#define MOVING_AVERAGE_SIZE 8

static int32_t approx_moving_average(int32_t data);
inline static float calculate_height(float pressure_initial, float pressure);
static float rate_limiter(float u, const float rising_rate,
                          const float falling_rate, const float Ts);

void task_state_est(void *argument) {

  log_debug("Task State Estimation started");

  static kalman_filter_t filter;

  kalman_init(&filter);

  /* Give the sensor read task time to initialize */
  osDelay(1000);
  float initial_pressure = (float)global_baro_data.pressure;
  float pressure_avarage = 0;
  uint32_t count = 0;

  uint32_t tick_count = osKernelGetTickCount();
  uint32_t tick_update = osKernelGetTickFreq() / SAMPLING_FREQ;
  while (1) {

    state_est_mode_e mode =
        osEventFlagsWait(state_est_mode_id, 0xFF, osFlagsWaitAny, 0);

    if (mode == STATE_EST_MODE_SHUTDOWN) {
      log_debug("Task State Estimation stopped");
      osThreadTerminate(osThreadGetId());
    }

    int32_t pressure = approx_moving_average(global_baro_data.pressure);

    /* Sanity check on pressure value */
    if (pressure > 0 && pressure < INT32_MAX && pressure > INT32_MIN) {
      /* Calculate the current altitude from the newest pressure measurement */
      float agl = calculate_height(initial_pressure, (float)pressure);

      /* Apply a rate limiter to reduce weird behavior at apogee */
      filter.altitude_agl =
          rate_limiter(agl, RATE_LIMIT_R, RATE_LIMIT_F, TIMESTEP);

      /* Run Kalman Filter */
      kalman_predict(&filter);
      kalman_update(&filter);

      /* Store Kalman estimate in global registers */
      global_state_data.altitude_agl = filter.x_bar[0];
      global_state_data.velocity = filter.x_bar[1];
    } else {
      global_state_data.altitude_agl = 0.0f;
      global_state_data.velocity = 0.0f;
    }

    /* Print out current altitude and velocity to USB for debugging purposes */
    log_info("Height %ld cm | Velocity %ld cm/s",
             (int32_t)(filter.x_hat[0] * 100.0f),
             (int32_t)(filter.x_hat[1] * 100.0f));

    if (global_flight_state >= ASCENT) {
      /* Record estimates for flight analysis */
      recorder_record(&recorder, osKernelGetTickCount(), REC_ALT_EST,
                      (int16_t)(global_state_data.altitude_agl * 100.0f));
      recorder_record(&recorder, osKernelGetTickCount(), REC_VEL_EST,
                      (int16_t)(global_state_data.velocity * 100.0f));
      recorder_record(&recorder, osKernelGetTickCount(), REC_ALT_FIL,
                      (int16_t)(filter.altitude_agl * 100.0f));
    } else {
      /* Reset the zero altitude every 20 samples as long as we are not in
       * flight*/
      if (count == 20) {
        initial_pressure = pressure_avarage / 20;
        pressure_avarage = 0;
        count = 0;
      } else {
        pressure_avarage += (float)global_baro_data.pressure;
        count++;
      }
    }

    tick_count += tick_update;
    osDelayUntil(tick_count);
  }
}

inline static float calculate_height(float pressure_initial, float pressure) {
  return (-(powf(pressure / pressure_initial, (1 / 5.257f)) - 1) *
          (TEMPERATURE_0 + 273.15f) / 0.0065f);
}

static int32_t approx_moving_average(int32_t data) {
  static int32_t avg = 0;

  avg -= avg / MOVING_AVERAGE_SIZE;
  avg += data / MOVING_AVERAGE_SIZE;

  return avg;
}

static float rate_limiter(float u, const float rising_rate,
                          const float falling_rate, const float Ts) {
  static float y_1 = 0;

  float rate = (u - y_1) / Ts;

  if (rate > rising_rate)
    y_1 = Ts * rising_rate + y_1;
  else if (rate < falling_rate)
    y_1 = Ts * falling_rate + y_1;
  else
    y_1 = u;

  return y_1;
}
