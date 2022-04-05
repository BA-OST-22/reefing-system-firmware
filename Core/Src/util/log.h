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

/* Logging inspired by https://github.com/rxi/log.c */

#pragma once

#include "cmsis_os.h"

#include <string.h>
#include <stdbool.h>


/** LOGGING SECTION **/

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

void log_init();
void log_set_level(int level);
void log_enable();
void log_disable();
bool log_is_enabled();

#define PRINT_BUFFER_LEN 420
extern osMutexId_t print_mutex;

void log_log(int level, const char *file, int line, const char *format, ...) __attribute__((format(printf, 4, 5)));

void log_raw(const char *format, ...) __attribute__((format(printf, 1, 2)));

/* just like log_raw, but without \n */
void log_rawr(const char *format, ...) __attribute__((format(printf, 1, 2)));


#if defined(_WIN32)
#define PATH_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

#define GET_FILENAME (strrchr(__FILE__, DIR_SEPARATOR) ? strrchr(__FILE__, DIR_SEPARATOR) + 1 : __FILE__)

#define log_trace(...) log_log(LOG_TRACE, GET_FILENAME, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, GET_FILENAME, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO, GET_FILENAME, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN, GET_FILENAME, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, GET_FILENAME, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, GET_FILENAME, __LINE__, __VA_ARGS__)
