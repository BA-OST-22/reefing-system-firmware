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

#include "eeprom.h"
#include <string.h>

#define PAGE 0
#define SECTOR 1
#define PAGE_NUM 2

#define _EE_USE_FLASH_PAGE_OR_SECTOR (1)
#define _EE_VOLTAGE FLASH_VOLTAGE_RANGE_3

#define _EE_SIZE (1024 * 16)
#define _EE_ADDR_INUSE                                                         \
  (((uint32_t)0x08000000) | (_EE_SIZE * _EE_USE_FLASH_PAGE_OR_SECTOR))
#define _EE_FLASH_BANK FLASH_BANK_1
#define _EE_VOLTAGE_RANGE _EE_VOLTAGE
#define _EE_PAGE_OR_SECTOR SECTOR

bool ee_init(void) { return true; }

bool ee_format() {
  uint32_t error;
  HAL_FLASH_Unlock();
  FLASH_EraseInitTypeDef flashErase;

  flashErase.NbSectors = 1;
  flashErase.Sector = _EE_USE_FLASH_PAGE_OR_SECTOR;
  flashErase.TypeErase = FLASH_TYPEERASE_SECTORS;
  flashErase.Banks = _EE_FLASH_BANK;
  flashErase.VoltageRange = _EE_VOLTAGE_RANGE;

  if (HAL_FLASHEx_Erase(&flashErase, &error) == HAL_OK) {
    HAL_FLASH_Lock();
    if (error != 0xFFFFFFFF)
      return false;
    else {
      return true;
    }
  }
  HAL_FLASH_Lock();
  return false;
}

bool ee_read(uint32_t startVirtualAddress, uint32_t len, uint8_t *data) {
  if ((startVirtualAddress + len) > _EE_SIZE)
    return false;
  for (uint32_t i = startVirtualAddress; i < len + startVirtualAddress; i++) {
    if (data != NULL) {
      *data = (*(uint8_t *)(i + _EE_ADDR_INUSE));
      data++;
    }
  }
  return true;
}

bool ee_write(uint32_t startVirtualAddress, uint32_t len, uint8_t *data) {
  if ((startVirtualAddress + len) > _EE_SIZE)
    return false;
  if (data == NULL)
    return false;
  HAL_FLASH_Unlock();
  for (uint32_t i = 0; i < len; i++) {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
                          ((i + startVirtualAddress)) + _EE_ADDR_INUSE,
                          (uint64_t)(data[i])) != HAL_OK) {
      HAL_FLASH_Lock();
      return false;
    }
  }
  HAL_FLASH_Lock();
  return true;
}

uint32_t ee_maxVirtualAddress(void) { return (_EE_SIZE); }
