/*
 * target.c
 *
 *  Created on: 8 Apr 2022
 *      Author: Luca
 */

#include "target.h"

ADC_HandleTypeDef hadc1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

