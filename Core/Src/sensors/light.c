/*
 * light.c
 *
 *  Created on: 13.04.2022
 *      Author: luca.jost
 */

#include "sensors/light.h"

void light_init(LIGHT_SENSOR *dev) { dev->dark = -1; }

/* Call this function when the sensor is obstructed,
 * the current brightness is stored as a reference for
 * the deployment */
void light_set_dark(LIGHT_SENSOR *dev) { dev->dark = adc_get(dev->source); }

void light_set_threshold(LIGHT_SENSOR *dev, uint32_t threshold) {
  dev->threshold = threshold;
}

light_state_e light_get_state(LIGHT_SENSOR *dev) {
  if (dev->dark == -1)
    return LIGHT_UNDEFINED;
  int32_t light = adc_get(dev->source);
  if ((light - dev->dark) > dev->threshold) {
    return LIGHT_BRIGHT;
  } else {
    return LIGHT_DARK;
  }
}

uint32_t light_get(LIGHT_SENSOR *dev) { return adc_get(dev->source); }
