#pragma once

#include "target/target.h"

static inline void pm_cut_on() {
  HAL_GPIO_WritePin(P_EN_CUT_GPIO_Port, P_EN_CUT_Pin, GPIO_PIN_SET);
}

static inline void pm_cut_off() {
  HAL_GPIO_WritePin(P_EN_CUT_GPIO_Port, P_EN_CUT_Pin, GPIO_PIN_RESET);
}

static inline void pm_radio_on() {
  HAL_GPIO_WritePin(P_EN_RADIO_GPIO_Port, P_EN_RADIO_Pin, GPIO_PIN_SET);
}

static inline void pm_radio_off() {
  HAL_GPIO_WritePin(P_EN_RADIO_GPIO_Port, P_EN_RADIO_Pin, GPIO_PIN_RESET);
}
