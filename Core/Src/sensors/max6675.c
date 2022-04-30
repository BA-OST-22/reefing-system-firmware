/*
 * max6675.c
 *
 *  Created on: 5 Apr 2022
 *      Author: Luca
 */

#include "max6675.h"

#define MAX_SPI_HANDLE hspi2

thermocouple_status_e get_temperature(float *temperature) {
  uint8_t tmp[2];
  HAL_GPIO_WritePin(TC_CS_GPIO_Port, TC_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Receive(&MAX_SPI_HANDLE, tmp, 2, 2);
  HAL_GPIO_WritePin(TC_CS_GPIO_Port, TC_CS_Pin, GPIO_PIN_SET);
  if (tmp[1] & 0x04) {
    return TMP_OPEN;
  } else {
    *temperature = (((uint16_t)(tmp[0]) << 5) + ((tmp[1] & 0xF8) >> 3)) / 4.0f;
    return TMP_OK;
  }
}
