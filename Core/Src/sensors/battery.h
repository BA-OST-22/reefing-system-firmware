#pragma once

#include "drivers/adc.h"

typedef enum {
  BATTERY_CHARGE = 1,
  BATTERY_DISCHARGE,
} battery_state_e;

typedef struct {
  adc_source_t source;
  float multiply;
  float add;
} BATTERY_SENSOR;

battery_state_e battery_voltage(BATTERY_SENSOR *dev, float *voltage);
