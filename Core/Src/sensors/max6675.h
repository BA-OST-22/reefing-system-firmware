
#pragma once

#include "target/target.h"

typedef enum{
	TMP_OK = 0,
	TMP_OPEN,
} thermocouple_status_t;

thermocouple_status_t get_temperature(float *temperature);
