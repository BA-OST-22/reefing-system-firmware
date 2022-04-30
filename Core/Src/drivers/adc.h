/*
 * adc.h
 *
 *  Created on: Mar 31, 2022
 *      Author: luca.jost
 */

#pragma once

#include "stm32f4xx_hal.h"

typedef enum {
  ADC_SOURCE_BATTERY = 0,
  ADC_SOURCE_LIGHT,
} adc_source_t;

uint32_t adc_get(adc_source_t channel);
