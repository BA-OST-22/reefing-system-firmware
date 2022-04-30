/*
 * battery.c
 *
 *  Created on: 17 Apr 2022
 *      Author: Luca
 */
#include "sensors/battery.h"
#include "target/target.h"

battery_state_e battery_voltage(BATTERY_SENSOR *dev, float *voltage) {
  float volt = (float)adc_get(dev->source);

  *voltage = volt * dev->multiply + dev->add;

  if (HAL_GPIO_ReadPin(CHRG_GPIO_Port, CHRG_Pin)) {
    return BATTERY_CHARGE;
  } else {
    return BATTERY_DISCHARGE;
  }
}
