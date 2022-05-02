/*
 * Reefing System Bachelor Thesis Software
 * Copyright (C) 2022 Institute for Microelectronics and Embedded Systems OST
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "stm32f4xx_hal.h"

#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define V_BAT_Pin GPIO_PIN_0
#define V_BAT_GPIO_Port GPIOA
#define LIGHT_Pin GPIO_PIN_1
#define LIGHT_GPIO_Port GPIOA
#define BUTTON_Pin GPIO_PIN_2
#define BUTTON_GPIO_Port GPIOA
#define BUTTON_EXTI_IRQn EXTI2_IRQn
#define TC_CS_Pin GPIO_PIN_3
#define TC_CS_GPIO_Port GPIOA
#define R_CS_Pin GPIO_PIN_4
#define R_CS_GPIO_Port GPIOA
#define RF_INT_Pin GPIO_PIN_0
#define RF_INT_GPIO_Port GPIOB
#define BARO_CS_Pin GPIO_PIN_1
#define BARO_CS_GPIO_Port GPIOB
#define IMU_CS_Pin GPIO_PIN_2
#define IMU_CS_GPIO_Port GPIOB
#define IMU_INT2_Pin GPIO_PIN_10
#define IMU_INT2_GPIO_Port GPIOB
#define IMU_INT2_EXTI_IRQn EXTI15_10_IRQn
#define IMU_INT1_Pin GPIO_PIN_12
#define IMU_INT1_GPIO_Port GPIOB
#define IMU_INT1_EXTI_IRQn EXTI15_10_IRQn
#define USB_DET_Pin GPIO_PIN_8
#define USB_DET_GPIO_Port GPIOA
#define CHRG_Pin GPIO_PIN_15
#define CHRG_GPIO_Port GPIOA
#define DCDC_EN_Pin GPIO_PIN_4
#define DCDC_EN_GPIO_Port GPIOB
#define CUT_EN_Pin GPIO_PIN_5
#define CUT_EN_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_6
#define BUZZER_GPIO_Port GPIOB
#define P_EN_LIGHT_Pin GPIO_PIN_7
#define P_EN_LIGHT_GPIO_Port GPIOB
#define P_EN_CUT_Pin GPIO_PIN_8
#define P_EN_CUT_GPIO_Port GPIOB
#define P_EN_RADIO_Pin GPIO_PIN_9
#define P_EN_RADIO_GPIO_Port GPIOB

extern ADC_HandleTypeDef hadc1;

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart1;

void Error_Handler(void);
