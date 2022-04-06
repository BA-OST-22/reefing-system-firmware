/*
 * This file was part of Cleanflight and Betaflight.
 * https://github.com/betaflight/betaflight
 * It is modified for the CATS Flight Software.
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

#include "config/config.h"
#include "cli/settings.h"

const char* const lookup_table_boot_state[] = {
    "CATS_INVALID", "CATS_IDLE", "CATS_CONFIG", "CATS_TIMER", "CATS_DROP", "CATS_FLIGHT",
};

const char* const lookup_table_events[] = {
    "MOVING", "READY", "LIFTOFF", "MAX_V", "APOGEE", "POST_APOGEE", "TOUCHDOWN", "CUSTOM_1", "CUSTOM_2",
};

const char* const lookup_table_actions[] = {
    "NONE",   "DELAY",   "HC_ONE",  "HC_TWO",    "HC_THREE",  "HC_FOUR",     "HC_FIVE",    "HC_SIX",   "LL_ONE",
    "LL_TWO", "LL_TREE", "LL_FOUR", "SERVO_ONE", "SERVO_TWO", "SERVO_THREE", "SERVO_FOUR", "RECORDER",
};

#define LOOKUP_TABLE_ENTRY(name) \
  { name, ARRAYLEN(name) }

const lookup_table_entry_t lookup_tables[] = {
    LOOKUP_TABLE_ENTRY(lookup_table_boot_state),
    LOOKUP_TABLE_ENTRY(lookup_table_events),
    LOOKUP_TABLE_ENTRY(lookup_table_actions),
};

#undef LOOKUP_TABLE_ENTRY

const cli_value_t value_table[] = {

    {"main_altitude", VAR_UINT16, .config.minmax_unsigned = {10, 65535},
     &global_config.config.main_altitude},
    {"acc_threshold", VAR_UINT16, .config.minmax_unsigned = {15, 80},
     &global_config.config.liftoff_acc_threshold},
};

const uint16_t value_table_entry_count = ARRAYLEN(value_table);
