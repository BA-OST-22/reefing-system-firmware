/*
 * CATS Flight Software
 * Copyright (C) 2021 Control and Telemetry Systems
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

#include "log.h"
#include "cmsis_os.h"
#include "config/globals.h"
#include "util/fifo.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static struct {
  int level;
  bool enabled;
} L;

static const char *level_strings[] = {"TRACE", "DEBUG", "INFO",
                                      "WARN",  "ERROR", "FATAL"};
static const char *level_colors[] = {"\x1b[94m", "\x1b[36m", "\x1b[32m",
                                     "\x1b[33m", "\x1b[31m", "\x1b[35m"};

osMutexId_t print_mutex;
static char print_buffer[PRINT_BUFFER_LEN];

void log_init() {
  const osMutexAttr_t print_mutex_attr = {
      "print_mutex",      // human readable mutex name
      osMutexPrioInherit, // attr_bits
      NULL,               // memory for control block
      0U                  // size for control block
  };
  print_mutex = osMutexNew(&print_mutex_attr);
}

void log_set_level(int level) { L.level = level; }

void log_enable() {
#if configUSE_TRACE_FACILITY == 0
  L.enabled = true;
#endif
}

void log_disable() { L.enabled = false; }

bool log_is_enabled() { return L.enabled; }

void log_log(int level, const char *file, int line, const char *format, ...) {
#if configUSE_TRACE_FACILITY == 0
  if (L.enabled && level >= L.level &&
      osMutexAcquire(print_mutex, 5U) == osOK) {
    /* fill buffer with metadata */
    static char buf_ts[16];
    buf_ts[snprintf(buf_ts, sizeof(buf_ts), "%lu", osKernelGetTickCount())] =
        '\0';
    static char buf_loc[30];
    buf_loc[snprintf(buf_loc, sizeof(buf_loc), "%s:%d:", file, line)] = '\0';
    int len;
    len = snprintf(print_buffer, PRINT_BUFFER_LEN,
                   "%6s %s%5s\x1b[0m \x1b[90m%30s\x1b[0m ", buf_ts,
                   level_colors[level], level_strings[level], buf_loc);
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(print_buffer + len, PRINT_BUFFER_LEN, format, argptr);
    va_end(argptr);
    snprintf(print_buffer + strlen(print_buffer), PRINT_BUFFER_LEN, "\n");
    fifo_write_bytes(&usb_output_fifo, (uint8_t *)print_buffer,
                     strlen(print_buffer));
    osMutexRelease(print_mutex);
  }
#endif
}

void log_raw(const char *format, ...) {
#if configUSE_TRACE_FACILITY == 0
  if (osMutexAcquire(print_mutex, 0U) == osOK) {
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(print_buffer, PRINT_BUFFER_LEN, format, argptr);
    va_end(argptr);
    snprintf(print_buffer + strlen(print_buffer), PRINT_BUFFER_LEN, "\n");
    fifo_write_bytes(&usb_output_fifo, (uint8_t *)print_buffer,
                     strlen(print_buffer));
    osMutexRelease(print_mutex);
  }
#endif
}

void log_rawr(const char *format, ...) {
#if configUSE_TRACE_FACILITY == 0
  if (osMutexAcquire(print_mutex, 0U) == osOK) {
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(print_buffer, PRINT_BUFFER_LEN, format, argptr);
    va_end(argptr);
    fifo_write_bytes(&usb_output_fifo, (uint8_t *)print_buffer,
                     strlen(print_buffer));
    osMutexRelease(print_mutex);
  }
#endif
}
