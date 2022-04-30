#include "config/eeprom.h"
#include <config/config.h>
#include <string.h>

const config_u DEFAULT_CONFIG = {
    .config.config_version = CONFIG_VERSION,
    .config.main_altitude = 150,
    .config.liftoff_acc_threshold = 35,
    .config.timer_duration = 0,
    .config.enable_telemetry = 0,
    .config.enable_preheat = 0,
    .config.buzzer_volume = 20,
};

config_u global_config = {};

void config_init() {}

void config_defaults() {
  memcpy(&global_config, &DEFAULT_CONFIG, sizeof(global_config));
}

/** persistence functions **/
void config_load() { ee_read(0, sizeof(config_t), global_config.config_array); }

bool config_save() {
  bool status = false;
  status = ee_format();
  if (status == false)
    return status;
  return ee_write(0, sizeof(config_t), global_config.config_array);
}
