/*
 * dcdc.h
 *
 *  Created on: Apr 4, 2022
 *      Author: Luca
 */
#pragma once

#include "target/target.h"

static inline void dcdc_enable(){
	/* Never turn on dcdc converter when usb is connected */
	if(HAL_GPIO_ReadPin(USB_DET_GPIO_Port, USB_DET_Pin) == 0){
		HAL_GPIO_WritePin(DCDC_EN_GPIO_Port, DCDC_EN_Pin, GPIO_PIN_SET);
	}
}

static inline void dcdc_disable(){
	HAL_GPIO_WritePin(DCDC_EN_GPIO_Port, DCDC_EN_Pin, GPIO_PIN_RESET);
}

void dcdc_set_voltage(float voltage);
float dcdc_get_voltage();
