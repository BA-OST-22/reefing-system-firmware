#pragma once

#include <stdbool.h>
#include <stdint.h>

/* The system will reload the default config when the number changes */
/* Config version 1 / Minor 0 */
#define CONFIG_VERSION 100

typedef struct {
  /* Needs to be in first position */
  uint32_t config_version;
  uint32_t main_altitude;
  uint32_t liftoff_acc_threshold;
  uint32_t timer_duration;
  uint8_t enable_telemetry;
  uint8_t enable_preheat;
  uint8_t link_phrase[10];
  uint8_t buzzer_volume;
} config_t;

typedef union {
  config_t config;
  uint8_t config_array[sizeof(config_t)];
} config_u;

extern config_u global_config;

/** cats config initialization **/
void config_init();
void config_defaults();

/** persistence functions **/
void config_load();
bool config_save();
