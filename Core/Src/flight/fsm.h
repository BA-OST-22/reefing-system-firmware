#pragma once

#include "config/globals.h"
#include "stdbool.h"
#include "stdint.h"

typedef struct {
  fsm_state_e flight_state;
  uint32_t memory[4];
  bool state_changed;
  uint32_t state_change_time;
} fsm_t;

void update_fsm(fsm_t *fsm);

#define READY_TIMEOUT 1000 // 10s

#define IDLE_DEEPSLEEP_TIMEOUT 6000 // 60s

#define SHORT_BUTTON_PRESS 10 // 0.1s
#define LONG_BUTTON_PRESS 300 // 3s
#define TIMEOUT_BETWEEN_BUTTON_PRESS 300 // 3s

#define LIFTOFF_SAMPLES 20 // 0.2s

#define APOGEE_SAMPLES 50 // 0.5s
#define MINIMUM_ASCENT_TIME 300 // 3s

#define PARACHUTE_OPENING_TIME 300 // 2s

#define BURN_TIME 20.0f // 20s
