/*
 * adc.h
 *
 *  Created on: Mar 31, 2022
 *      Author: luca.jost
 */

#pragma once

#include "stm32f4xx_hal.h"

typedef enum {
	BATTERY = 0,
	LIGHT,
}adc_source_t;

uint32_t ADC_Get(adc_source_t channel);


