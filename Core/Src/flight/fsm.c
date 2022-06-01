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

#include "fsm.h"
#include "config/config.h"
#include "config/globals.h"
#include "drivers/led.h"
#include "drivers/sleep.h"
#include "flight/state_transition.h"
#include "target/target.h"
#include "util/log.h"

static void check_idle_state(fsm_t *fsm);
static void check_deepsleep_state(fsm_t *fsm);
static void check_ready_state(fsm_t *fsm);
static void check_ascent_state(fsm_t *fsm);
static void check_descent_state(fsm_t *fsm);
static void check_deplyoment_state(fsm_t *fsm);
static void check_recovery_state(fsm_t *fsm);

const char *const state_name[] = {
    "INVALID", "IDLE",    "DEEP_SLEEP", "READY",
    "ASCENT",  "DESCENT", "DEPLOYMENT", "RECOVERY",
};

/**
 * @brief Updates the FSM state
 *
 * @param  fsm: pointer to the FSM structure
 * @retval None
 */
void update_fsm(fsm_t *fsm) {
  fsm_state_e old_state = fsm->flight_state;
  switch (fsm->flight_state) {
  case IDLE:
    check_idle_state(fsm);
    break;
  case DEEP_SLEEP:
    check_deepsleep_state(fsm);
    break;
  case READY:
    check_ready_state(fsm);
    break;
  case ASCENT:
    check_ascent_state(fsm);
    break;
  case DESCENT:
    check_descent_state(fsm);
    break;
  case DEPLOYMENT:
    check_deplyoment_state(fsm);
    break;
  case RECOVERY:
    check_recovery_state(fsm);
    break;
  default:
    break;
  }

  if (old_state != fsm->flight_state) {
    fsm->state_changed = true;
    fsm->state_change_time = osKernelGetTickCount();
    recorder_record(&recorder, fsm->state_change_time, REC_EVENT,
                    (int16_t)fsm->flight_state);
    log_warn("State Transition %s to %s", state_name[old_state],
             state_name[fsm->flight_state]);
  }
}

/**
 * @brief Checks the IDLE state
 *
 * @param  fsm: pointer to the FSM structure
 * @retval None
 */
static void check_idle_state(fsm_t *fsm) {

  /* When button is pressed */
  if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0) {
    led_off();
    fsm->memory[3] = 0;
    if (fsm->memory[1] == 0) {
      fsm->memory[0]++;
    } else {
      fsm->memory[2]++;
    }
  }
  /* When button is not pressed */
  else {
    /* Count timeout */
    fsm->memory[3]++;
    /* Short button press */
    if (fsm->memory[0] > SHORT_BUTTON_PRESS) {
      led_on();
      fsm->memory[1]++; // Count timeout between button press
    }
    /* Button bounce or no press */
    else {
      fsm->memory[0] = 0;
      fsm->memory[0] = 0;
      fsm->memory[2] = 0;
    }
  }

  /* Long button press */
  if (fsm->memory[0] > LONG_BUTTON_PRESS) {
    /* State Transition to READY */
    fsm->memory[0] = 0;
    fsm->memory[1] = 0;
    fsm->memory[2] = 0;
    fsm->memory[3] = 0;
    fsm->flight_state = READY;
    state_transition_idle_ready();
  }

  /* Long press after short press */
  if (fsm->memory[2] > LONG_BUTTON_PRESS) {
    /* State Transition to DEEP_SLEEP */
    fsm->memory[0] = 0;
    fsm->memory[1] = 0;
    fsm->memory[2] = 0;
    fsm->memory[3] = 0;
    fsm->flight_state = DEEP_SLEEP;
    state_transition_idle_deepsleep();
  }

  /* Timeout after short press */
  if (fsm->memory[1] > TIMEOUT_BETWEEN_BUTTON_PRESS) {
    led_off();
    fsm->memory[0] = 0;
    fsm->memory[1] = 0;
    fsm->memory[2] = 0;
    fsm->memory[3] = 0;
  }

  /* After long timeout go to deepsleep to preserve battery life */
  if (fsm->memory[3] > IDLE_DEEPSLEEP_TIMEOUT) {
    fsm->memory[0] = 0;
    fsm->memory[1] = 0;
    fsm->memory[2] = 0;
    fsm->memory[3] = 0;
    if (HAL_GPIO_ReadPin(USB_DET_GPIO_Port, USB_DET_Pin) == 0) {
      fsm->flight_state = DEEP_SLEEP;
      state_transition_idle_deepsleep();
    }
  }
}

/**
 * @brief Checks the DEEP_SLEEP state
 *
 * @param  fsm: pointer to the FSM structure
 * @retval None
 */
static void check_deepsleep_state(fsm_t *fsm) {
  led_on();
  /* When button is pressed */
  if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0) {
    if (fsm->memory[1] == 0) {
      fsm->memory[0]++;
    } else {
      fsm->memory[2]++;
    }
  }
  /* When button is not pressed */
  else {
    fsm->memory[1]++; // Count timeout between button press
  }

  /* Long press after short press */
  if (fsm->memory[2] > LONG_BUTTON_PRESS) {
    /* State Transition to IDLE */
    fsm->memory[0] = 0;
    fsm->memory[1] = 0;
    fsm->memory[2] = 0;
    fsm->flight_state = IDLE;
    state_transition_deepsleep_idle();
  }

  /* Timeout after short press */
  if (fsm->memory[1] > TIMEOUT_BETWEEN_BUTTON_PRESS) {
    fsm->memory[0] = 0;
    fsm->memory[1] = 0;
    fsm->memory[2] = 0;
    /* Go back to sleep */
    go_to_sleep(WAKEUP_BUTTON);
  }
}

/**
 * @brief Checks the READY state
 *
 * @param  fsm: pointer to the FSM structure
 * @retval None
 */
static void check_ready_state(fsm_t *fsm) {
  led_on();
  /* When button is pressed */
  if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0) {
    fsm->memory[1] = 0;
    fsm->memory[0]++;
  }
  /* When button is not pressed */
  else {
    fsm->memory[1]++; // Count timeout
    fsm->memory[0] = 0;
  }

  /* Long button press */
  if (fsm->memory[0] > LONG_BUTTON_PRESS) {
    /* State Transition to IDLE */
    fsm->memory[0] = 0;
    fsm->memory[1] = 0;
    fsm->memory[2] = 0;
    fsm->flight_state = IDLE;
    state_transition_ready_idle();
  }

  if (fsm->memory[1] > READY_TIMEOUT) {
    /* Go to sleep */
    fsm->memory[0] = 0;
    fsm->memory[1] = 0;
    fsm->memory[2] = 0;
    /* Only sleep when no USB is connected */
    if (HAL_GPIO_ReadPin(USB_DET_GPIO_Port, USB_DET_Pin) == 0) {
      led_off();
      go_to_sleep(WAKEUP_BOTH);
    }
  }

  /* Acceleration above threshold */
  if ((global_imu_data.acceleration[0] * global_imu_data.acceleration[0] +
       global_imu_data.acceleration[1] * global_imu_data.acceleration[1] +
       global_imu_data.acceleration[2] * global_imu_data.acceleration[2]) >
      global_config.config.liftoff_acc_threshold *
          global_config.config.liftoff_acc_threshold) {
    fsm->memory[2]++;
  } else {
    fsm->memory[2] = 0;
  }

  /* Liftoff detected */
  if (fsm->memory[2] > LIFTOFF_SAMPLES) {
    fsm->memory[0] = 0;
    fsm->memory[1] = 0;
    fsm->memory[2] = 0;
    led_off();
    fsm->flight_state = ASCENT;
    state_transition_ready_ascent();
  }
}

/**
 * @brief Checks the ASCENT state
 *
 * @param  fsm: pointer to the FSM structure
 * @retval None
 */
static void check_ascent_state(fsm_t *fsm) {
  if ((fsm->state_change_time + (MINIMUM_ASCENT_TIME * 10)) >
      osKernelGetTickCount())
    return;
  /* When negative velocity is estimated */
  if (global_state_data.velocity < 0) {
    fsm->memory[0]++;
  }
  /* When light sensor detects light */
  if (global_light == true) {
    fsm->memory[1]++;
  }

  /* State transition to descent when velocity is negative or light is detected
   */
  if ((fsm->memory[0] > APOGEE_SAMPLES) || (fsm->memory[1] > APOGEE_SAMPLES)) {
    fsm->memory[0] = 0;
    fsm->memory[1] = 0;
    fsm->memory[2] = 0;
    fsm->flight_state = DESCENT;
    state_transition_ascent_descent();
  }
}

/**
 * @brief Checks the DESCENT state
 *
 * @param  fsm: pointer to the FSM structure
 * @retval None
 */
static void check_descent_state(fsm_t *fsm) {
  if ((fsm->state_change_time + ((uint32_t)global_config.config.apogee_delay *
                                 1000)) > osKernelGetTickCount())
    return;
  float est_alt_at_deployment =
      (global_state_data.velocity * global_config.config.burn_duration) +
      global_state_data.altitude_agl;

  if ((float)global_config.config.main_altitude >= est_alt_at_deployment) {
    fsm->flight_state = DEPLOYMENT;
    state_transition_descent_deployment();
  }
}

/**
 * @brief Checks the DEPLOYMENT state
 *
 * @param  fsm: pointer to the FSM structure
 * @retval None
 */
static void check_deplyoment_state(fsm_t *fsm) {
  if ((fsm->state_change_time + ((global_config.config.burn_duration + 5.0f) *
                                 1000)) > osKernelGetTickCount())
    return;
  fsm->flight_state = RECOVERY;
  state_transition_deployment_recovery();
}

/**
 * @brief Checks the RECOVERY state
 *
 * @param  fsm: pointer to the FSM structure
 * @retval None
 */
static void check_recovery_state(fsm_t *fsm) {
  /* When button is pressed */
  if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0) {
    fsm->memory[0]++;
  } else {
    if (fsm->memory[0] > SHORT_BUTTON_PRESS) {
      fsm->flight_state = IDLE;
      fsm->memory[0] = 0;
      state_transition_recovery_idle();
    }
    fsm->memory[0] = 0;
  }
}
