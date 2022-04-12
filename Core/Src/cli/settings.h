/*
 * This file was part of Cleanflight and Betaflight.
 * https://github.com/betaflight/betaflight
 * It is modified for the Reefing System Firmware.
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

#include "config/config.h"

#include <stdbool.h>

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

typedef enum { TABLE_BOOTSTATE = 0, TABLE_EVENTS, TABLE_ACTIONS, TABLE_SPEEDS } lookup_table_index_e;

typedef struct {
  const char *const *values;
  const uint8_t value_count;
} lookup_table_entry_t;

#define VALUE_TYPE_OFFSET    0
#define VALUE_SECTION_OFFSET 3
#define VALUE_MODE_OFFSET    5

typedef enum {
  // value type, bits 0-2
  VAR_UINT8 = (0 << VALUE_TYPE_OFFSET),
  VAR_INT8 = (1 << VALUE_TYPE_OFFSET),
  VAR_UINT16 = (2 << VALUE_TYPE_OFFSET),
  VAR_INT16 = (3 << VALUE_TYPE_OFFSET),
  VAR_UINT32 = (4 << VALUE_TYPE_OFFSET),

  // value mode, bits 5-7
  MODE_DIRECT = (0 << VALUE_MODE_OFFSET),
  MODE_LOOKUP = (1 << VALUE_MODE_OFFSET),
  MODE_ARRAY = (2 << VALUE_MODE_OFFSET),
  MODE_BITSET = (3 << VALUE_MODE_OFFSET),
  MODE_STRING = (4 << VALUE_MODE_OFFSET),
} cli_value_flag_e;

#define VALUE_TYPE_MASK    (0x07)
#define VALUE_SECTION_MASK (0x18)
#define VALUE_MODE_MASK    (0xE0)

typedef struct {
  const int16_t min;
  const int16_t max;
} cli_minmax_config_t;

typedef struct {
  const uint16_t min;
  const uint16_t max;
} cli_minmax_unsigned_config_t;

typedef struct {
  const lookup_table_index_e table_index;
} cli_lookup_table_config_t;

typedef struct {
  const uint8_t length;
} cli_array_length_config_t;

typedef struct {
  const uint8_t min_length;
  const uint8_t max_length;
  const uint8_t flags;
} cli_string_length_config_t;

#define STRING_FLAGS_NONE      (0)
#define STRING_FLAGS_WRITEONCE (1 << 0)

typedef union {
  cli_lookup_table_config_t lookup;              // used for MODE_LOOKUP excl. VAR_UINT32
  cli_minmax_config_t minmax;                    // used for MODE_DIRECT with signed parameters
  cli_minmax_unsigned_config_t minmax_unsigned;  // used for MODE_DIRECT with unsigned parameters
  cli_array_length_config_t array;               // used for MODE_ARRAY
  cli_string_length_config_t string;             // used for MODE_STRING
  uint8_t bitpos;                                // used for MODE_BITSET
  uint32_t u32_max;                              // used for MODE_DIRECT with VAR_UINT32
} cli_value_config_t;

struct cli_value;
typedef void (*callback_f)(const struct cli_value *arg);

typedef struct cli_value {
  const char *name;
  const uint8_t type;  // see cli_value_flag_e
  const cli_value_config_t config;
  void *pdata;
  callback_f cb;
} __attribute__((packed)) cli_value_t;

extern const lookup_table_entry_t lookup_tables[];
extern const uint16_t value_table_entry_count;

extern const cli_value_t value_table[];
