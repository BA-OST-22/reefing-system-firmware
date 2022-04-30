
#pragma once

#include "target/target.h"

typedef enum {
  TMP_OK = 0,
  TMP_OPEN,
} thermocouple_status_e;

thermocouple_status_e get_temperature(float *temperature);
