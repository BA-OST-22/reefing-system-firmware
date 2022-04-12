/*
 * This file was adapted from Cleanflight and Betaflight.
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

#include "cli/cli.h"
#include "cli/cli_commands.h"
#include "util/log.h"
#include "config/globals.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/** CLI command function declarations **/
static void cli_cmd_help(const char *cmd_name, char *args);
static void cli_cmd_reboot(const char *cmd_name, char *args);
static void cli_cmd_save(const char *cmd_name, char *args);

static void cli_cmd_get(const char *cmd_name, char *args);
static void cli_cmd_set(const char *cmd_name, char *args);
static void cli_cmd_defaults(const char *cmd_name, char *args);
static void cli_cmd_dump(const char *cmd_name, char *args);

static void cli_cmd_status(const char *cmd_name, char *args);
static void cli_cmd_version(const char *cmd_name, char *args);

static void cli_cmd_log_enable(const char *cmd_name, char *args);


/* List of CLI commands; should be sorted in alphabetical order. */
const clicmd_t cmd_table[] = {
    CLI_COMMAND_DEF("defaults", "reset to defaults and reboot", NULL, cli_cmd_defaults),
    CLI_COMMAND_DEF("dump", "Dump configuration", NULL, cli_cmd_dump),
    CLI_COMMAND_DEF("get", "get variable value", "[cmd_name]", cli_cmd_get),
    CLI_COMMAND_DEF("help", "display command help", "[search string]", cli_cmd_help),
    CLI_COMMAND_DEF("log_enable", "enable the logging output", NULL, cli_cmd_log_enable),
    CLI_COMMAND_DEF("reboot", "reboot without saving", NULL, cli_cmd_reboot),
    CLI_COMMAND_DEF("save", "save configuration", NULL, cli_cmd_save),
    CLI_COMMAND_DEF("set", "change setting", "[<cmd_name>=<value>]", cli_cmd_set),
    CLI_COMMAND_DEF("status", "show status", NULL, cli_cmd_status),
    CLI_COMMAND_DEF("version", "show version", NULL, cli_cmd_version),
};

const size_t NUM_CLI_COMMANDS = sizeof cmd_table / sizeof cmd_table[0];

static const char* const emptyName = "-";
static const char* const emptyString = "";

/** Helper function declarations **/

static void cli_set_var(const cli_value_t *var, uint32_t value);

/** CLI command function definitions **/

static void cli_cmd_help(const char *cmd_name, char *args) {
  bool any_matches = false;

  for (uint32_t i = 0; i < ARRAYLEN(cmd_table); i++) {
    bool print_entry = false;
    if (is_empty(args)) {
      print_entry = true;
    } else {
      if (strstr(cmd_table[i].name, args) || strstr(cmd_table[i].description, args)) {
        print_entry = true;
      }
    }

    if (print_entry) {
      any_matches = true;
      cli_print(cmd_table[i].name);
      if (cmd_table[i].description) {
        cli_printf(" - %s", cmd_table[i].description);
      }
      if (cmd_table[i].args) {
        cli_printf("\r\n\t%s", cmd_table[i].args);
      }
      cli_print_linefeed();
    }
  }
  if (!is_empty(args) && !any_matches) {
    cli_print_error_linef(cmd_name, "NO MATCHES FOR '%s'", args);
  }
}

static void cli_cmd_reboot(const char *cmd_name, char *args) { NVIC_SystemReset(); }

static void cli_cmd_save(const char *cmd_name, char *args) {
  if (config_save() == false) {
    cli_print_line("Saving unsuccessful, trying force save...");
    if (config_save() == false) {
      cli_print_line("Force save failed!");
      return;
    }
  }
  cli_print_line("Successfully written to flash");
  osEventFlagsSet(buzzer_event_id, BEEP_OK);
}

static void cli_cmd_get(const char *cmd_name, char *args) {
  const cli_value_t *val;
  int matched_commands = 0;

  for (uint32_t i = 0; i < value_table_entry_count; i++) {
    if (strstr(value_table[i].name, args)) {
      val = &value_table[i];
      if (matched_commands > 0) {
        cli_print_linefeed();
      }
      cli_printf("%s = ", value_table[i].name);
      cli_print_var(cmd_name, val, 0);
      cli_print_linefeed();
      cli_print_var_range(val);
      // cliPrintVarDefault(cmd_name, val);

      matched_commands++;
    }
  }

  if (!matched_commands) {
    cli_print_error_linef(cmd_name, "INVALID NAME");
  }
}

static void cli_cmd_set(const char *cmd_name, char *args) {
  const uint32_t len = strlen(args);
  char *eqptr;

  if (len == 0 || (len == 1 && args[0] == '*')) {
    cli_print_line("Current settings: ");

    for (uint32_t i = 0; i < value_table_entry_count; i++) {
      const cli_value_t *val = &value_table[i];
      cli_printf("%s = ", value_table[i].name);
      // when len is 1 (when * is passed as argument), it will print min/max values as well, for gui
      cli_print_var(cmd_name, val, len);
      cli_print_linefeed();
    }
  } else if ((eqptr = strstr(args, "=")) != NULL) {
    // has equals

    uint8_t variable_name_length = get_word_length(args, eqptr);

    // skip the '=' and any ' ' characters
    eqptr++;
    eqptr = skip_space(eqptr);

    const uint16_t index = cli_get_setting_index(args, variable_name_length);
    if (index >= value_table_entry_count) {
      cli_print_error_linef(cmd_name, "INVALID NAME");
      return;
    }
    const cli_value_t *val = &value_table[index];

    bool value_changed = false;

    switch (val->type & VALUE_MODE_MASK) {
      case MODE_DIRECT: {
        if ((val->type & VALUE_TYPE_MASK) == VAR_UINT32) {
          uint32_t value = strtoul(eqptr, NULL, 10);

          if (value <= val->config.u32_max) {
            cli_set_var(val, value);
            value_changed = true;
          }
        } else {
          int value = atoi(eqptr);

          int min;
          int max;
          get_min_max(val, &min, &max);

          if (value >= min && value <= max) {
            cli_set_var(val, value);
            value_changed = true;
          }
        }
      }

      break;
      case MODE_LOOKUP:
      case MODE_BITSET: {
        int tableIndex;
        if ((val->type & VALUE_MODE_MASK) == MODE_BITSET) {
          tableIndex = TABLE_BOOTSTATE;
        } else {
          tableIndex = val->config.lookup.table_index;
        }
        const lookup_table_entry_t *tableEntry = &lookup_tables[tableIndex];
        bool matched = false;
        for (uint32_t tableValueIndex = 0; tableValueIndex < tableEntry->value_count && !matched; tableValueIndex++) {
          matched = tableEntry->values[tableValueIndex] && strcasecmp(tableEntry->values[tableValueIndex], eqptr) == 0;

          if (matched) {
            cli_set_var(val, tableValueIndex);
            value_changed = true;
          }
        }
      } break;
      case MODE_ARRAY: {
        const uint8_t array_length = val->config.array.length;
        char *valPtr = eqptr;

        int i = 0;
        while (i < array_length && valPtr != NULL) {
          // skip spaces
          valPtr = skip_space(valPtr);

          // process substring starting at valPtr
          // note: no need to copy substrings for atoi()
          //       it stops at the first character that cannot be converted...
          switch (val->type & VALUE_TYPE_MASK) {
            default:
            case VAR_UINT8: {
              // fetch data pointer
              uint8_t *data = (uint8_t *)val->pdata + i;
              // store value
              *data = (uint8_t)atoi((const char *)valPtr);
            }

            break;
            case VAR_INT8: {
              // fetch data pointer
              int8_t *data = (int8_t *)val->pdata + i;
              // store value
              *data = (int8_t)atoi((const char *)valPtr);
            }

            break;
            case VAR_UINT16: {
              // fetch data pointer
              uint16_t *data = (uint16_t *)val->pdata + i;
              // store value
              *data = (uint16_t)atoi((const char *)valPtr);
            }

            break;
            case VAR_INT16: {
              // fetch data pointer
              int16_t *data = (int16_t *)val->pdata + i;
              // store value
              *data = (int16_t)atoi((const char *)valPtr);
            }

            break;
            case VAR_UINT32: {
              // fetch data pointer
              uint32_t *data = (uint32_t *)val->pdata + i;
              // store value
              *data = (uint32_t)strtoul((const char *)valPtr, NULL, 10);
            }

            break;

          }

          // find next comma (or end of string)
          valPtr = strchr(valPtr, ',') + 1;

          i++;
        }
      }
        // mark as changed
        value_changed = true;

        break;
      case MODE_STRING: {
		  char *valPtr = eqptr;
		  valPtr = skip_space(valPtr);

		  const unsigned int len = strlen(valPtr);
		  const uint8_t min = val->config.string.min_length;
		  const uint8_t max = val->config.string.max_length;
		  const bool updatable = ((val->config.string.flags & STRING_FLAGS_WRITEONCE) == 0 ||
								  strlen((char *)val->pdata) == 0 ||
								  strncmp(valPtr, (char *)val->pdata, len) == 0);

		  if (updatable && len > 0 && len <= max) {
			  memset((char *)val->pdata, 0, max);
			  if (len >= min && strncmp(valPtr, emptyName, len)) {
				  strncpy((char *)val->pdata, valPtr, len);
			  }
			  value_changed = true;
		  } else {
			  cli_print_error_linef(cmd_name, "STRING MUST BE 1-%d CHARACTERS OR '-' FOR EMPTY", max);
		  }
	  }
	  break;
    }

    if (value_changed) {
      cli_printf("%s set to ", val->name);
      cli_print_var(cmd_name, val, 0);
      if (val->cb != NULL) {
        val->cb(val);
      }
    } else {
      cli_print_error_linef(cmd_name, "INVALID VALUE");
      cli_print_var_range(val);
    }

    return;
  } else {
    // no equals, check for matching variables.
    cli_cmd_get(cmd_name, args);
  }
}


static void cli_cmd_defaults(const char *cmd_name, char *args) {
  config_defaults();
  cli_print_line("Reset to default values");
}

static void cli_cmd_dump(const char *cmd_name, char *args) {
  const uint32_t len = strlen(args);
  cli_printf("#Configuration dump");
  cli_print_linefeed();
  for (uint32_t i = 0; i < value_table_entry_count; i++) {
    const cli_value_t *val = &value_table[i];
    cli_printf("set %s = ", value_table[i].name);
    // when len is 1 (when * is passed as argument), it will print min/max values as well
    cli_print_var(cmd_name, val, len);
    cli_print_linefeed();
  }
  cli_printf("#End of configuration dump");
}

static void cli_cmd_status(const char *cmd_name, char *args) {
  //const lookup_table_entry_t *p_boot_table = &lookup_tables[TABLE_BOOTSTATE];
  //const lookup_table_entry_t *p_event_table = &lookup_tables[TABLE_EVENTS];
  cli_printf("System time: %lu ticks\n", osKernelGetTickCount());
}

static void cli_cmd_version(const char *cmd_name, char *args) {
  /* TODO: Store the board name somewhere else. */
  cli_printf("Board: %s\n", "Reefing System");
  cli_printf("CPU ID: 0x%lx, Revision: 0x%lx\n", HAL_GetDEVID(), HAL_GetREVID());
}

static void cli_cmd_log_enable(const char *cmd_name, char *args) { log_enable(); }


static void cli_set_var(const cli_value_t *var, const uint32_t value) {
  void *ptr = var->pdata;
  uint32_t work_value;
  uint32_t mask;

  if ((var->type & VALUE_MODE_MASK) == MODE_BITSET) {
    switch (var->type & VALUE_TYPE_MASK) {
      case VAR_UINT8:
        mask = (1 << var->config.bitpos) & 0xff;
        if (value) {
          work_value = *(uint8_t *)ptr | mask;
        } else {
          work_value = *(uint8_t *)ptr & ~mask;
        }
        *(uint8_t *)ptr = work_value;
        break;

      case VAR_UINT16:
        mask = (1 << var->config.bitpos) & 0xffff;
        if (value) {
          work_value = *(uint16_t *)ptr | mask;
        } else {
          work_value = *(uint16_t *)ptr & ~mask;
        }
        *(uint16_t *)ptr = work_value;
        break;

      case VAR_UINT32:
        mask = 1 << var->config.bitpos;
        if (value) {
          work_value = *(uint32_t *)ptr | mask;
        } else {
          work_value = *(uint32_t *)ptr & ~mask;
        }
        *(uint32_t *)ptr = work_value;
        break;
    }
  } else {
    switch (var->type & VALUE_TYPE_MASK) {
      case VAR_UINT8:
        *(uint8_t *)ptr = value;
        break;

      case VAR_INT8:
        *(int8_t *)ptr = value;
        break;

      case VAR_UINT16:
        *(uint16_t *)ptr = value;
        break;

      case VAR_INT16:
        *(int16_t *)ptr = value;
        break;

      case VAR_UINT32:
        *(uint32_t *)ptr = value;
        break;
    }
  }
}
