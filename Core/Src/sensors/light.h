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
