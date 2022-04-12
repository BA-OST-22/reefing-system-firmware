#pragma once

#include "stm32f4xx_hal.h"

void init();

void SystemClock_Config(void);
void init_cli();
void init_idle();
void init_deepsleep();
