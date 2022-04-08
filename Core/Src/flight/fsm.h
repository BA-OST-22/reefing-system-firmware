#pragma once

#include "stdint.h"
#include "stdbool.h"



typedef enum {
  INVALID = 0,
  IDLE = 1,
  DEEP_SLEEP,
  READY,
  READY_SLEEP,
  ASCENT,
  DESCENT,
  DEPLOYMENT,
  RECOVERY,
} fsm_state_e;

typedef struct {
  fsm_state_e flight_state;
  uint32_t memory[3];
  bool state_changed;
} fsm_t;

void update_fsm(fsm_t* fsm);


#define SHORT_BUTTON_PRESS 50
#define LONG_BUTTON_PRESS 300
#define TIMEOUT_BETWEEN_BUTTON_PRESS 300
