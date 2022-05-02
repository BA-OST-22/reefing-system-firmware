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

#include "buzzer.h"
#include "cmsis_os2.h"

void buzzer_beep(BUZ_DEV *dev, uint32_t duration) {
  dev->end_time = osKernelGetTickCount() + duration;
  dev->start = 1;
}

// Set the volume between 0 and 100
void buzzer_set_volume(BUZ_DEV *dev, uint16_t volume) {
  if (volume > 100)
    volume = 100;

  TIM_OC_InitTypeDef sConfigOC;
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = (dev->arr / 200) * volume; // set the pulse duration
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(dev->timer, &sConfigOC, dev->channel);

  // Start the pwm channel again if it should be running
  if (dev->started)
    HAL_TIM_PWM_Start(dev->timer, dev->channel);
  else
    HAL_TIM_PWM_Stop(dev->timer, dev->channel);

  dev->volume = volume;
}

// Set buzzer frequency between 200 and 10kHz
void buzzer_set_freq(BUZ_DEV *dev, uint32_t frequency) {
  // FREQ = CORE_FREQ / ((AAR+1) * (PSC+1))
  uint32_t core_freq = HAL_RCC_GetHCLKFreq();
  uint32_t psc = 1;
  // guards
  if (frequency > 10000)
    frequency = 10000;
  else if (frequency < 200)
    frequency = 200;

  dev->arr = (uint16_t)(core_freq / (frequency * psc + frequency)) - 1;

  // Update timer period
  dev->timer->Init.Period = dev->arr;
  dev->timer->Init.Prescaler = psc;
  HAL_TIM_PWM_Init(dev->timer);

  // Update pulse as the freq changed
  buzzer_set_volume(dev, dev->volume);
}

// Starts pwm timer
void buzzer_start(BUZ_DEV *dev) {
  dev->started = 1;
  HAL_TIM_PWM_Start(dev->timer, dev->channel); // start pwm generation
}

// Stops pwm timer
void buzzer_stop(BUZ_DEV *dev) {
  dev->started = 0;
  HAL_TIM_PWM_Stop(dev->timer, dev->channel); // stop pwm generation
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}

// Starts and Stops the buzzer
// Returns 1 if buzzer is running
uint8_t buzzer_update(BUZ_DEV *dev) {
  if (dev->start) {
    buzzer_start(dev);
    dev->start = 0;
  }
  if (dev->started && (dev->end_time > osKernelGetTickCount()))
    return 1;
  else if (dev->started)
    buzzer_stop(dev);
  return 0;
}
