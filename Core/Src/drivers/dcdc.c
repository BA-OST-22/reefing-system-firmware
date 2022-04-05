/*
 * dcdc.c
 *
 *  Created on: Apr 4, 2022
 *      Author: Luca
 */
#include "math.h"
#include "dcdc.h"

#define DCDC_SPI_HANDLE hspi2

static float voltage = 0;

void dcdc_set_voltage(float volt){
	uint8_t r_value;
	if(volt > 12.2f) volt = 12.2f;
	else if(volt < 5.2f) volt = 5.2f;

	voltage = volt;

	// Cubic least square fit
	// https://www.wolframalpha.com/input?i=cubic+fit+%7B12.2%2C0%7D%2C%7B11.48%2C10%7D%2C%7B8.83%2C64%7D%2C%7B7.04%2C128%7D%2C%7B6.43%2C160%7D%2C%7B5.21%2C255%7D
	r_value = -0.791728f * powf(voltage, 3) + 25.66f * powf(voltage, 2) - 293.662f * voltage + 1199.67f;

	HAL_GPIO_WritePin(R_CS_GPIO_Port, R_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&DCDC_SPI_HANDLE, &r_value, 1, 2);
	HAL_GPIO_WritePin(R_CS_GPIO_Port, R_CS_Pin, GPIO_PIN_SET);
}

float dcdc_get_voltage(){
	return voltage;
}
