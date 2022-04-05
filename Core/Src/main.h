/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
extern SPI_HandleTypeDef hspi2;

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void SystemClock_Config(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
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
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
