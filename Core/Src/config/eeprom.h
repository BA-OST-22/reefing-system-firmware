/*
  Author:     Nima Askari
  WebSite:    http://www.github.com/NimaLTD
  Instagram:  http://instagram.com/github.NimaLTD
  Youtube:    https://www.youtube.com/channel/UCUhY7qY1klJm1d2kulr9ckw
  
  Version:    2.0.3

  (2.0.3)
  Add F411.
  
  (2.0.2)
  Add L4.  
  
  (2.0.1)
  Change function name to ee_commit().
  
  Reversion History:
  (2.0.0)
  Rewrite again.

*/

#pragma once

#include <stdbool.h>
#include "main.h"

bool      ee_init(void);
bool      ee_format();
bool      ee_read(uint32_t startVirtualAddress, uint32_t len, uint8_t* data);
bool      ee_write(uint32_t startVirtualAddress, uint32_t len, uint8_t* data);
uint32_t  ee_maxVirtualAddress(void);
