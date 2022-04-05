/*
 * dcdc.h
 *
 *  Created on: Apr 4, 2022
 *      Author: Luca
 */
#pragma once

#include "main.h"

static inline void dcdc_enable(){
	HAL_GPIO_WritePin(DCDC_EN_GPIO_Port, DCDC_EN_Pin, GPIO_PIN_SET);
}

static inline void dcdc_disable(){
	HAL_GPIO_WritePin(DCDC_EN_GPIO_Port, DCDC_EN_Pin, GPIO_PIN_RESET);
}

void dcdc_set_voltage(float voltage);
float dcdc_get_voltage();
