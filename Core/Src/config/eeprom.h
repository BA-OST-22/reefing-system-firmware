

#pragma once

#include <stdbool.h>
#include "main.h"

bool      ee_init(void);
bool      ee_format();
bool      ee_read(uint32_t startVirtualAddress, uint32_t len, uint8_t* data);
bool      ee_write(uint32_t startVirtualAddress, uint32_t len, uint8_t* data);
uint32_t  ee_maxVirtualAddress(void);
