#pragma once

#include "target/target.h"
#include <stdbool.h>

typedef struct {
  uint32_t size;
  uint32_t index;
  uint8_t *data;
  uint32_t first_ts;
  uint32_t element_count;
} recorder_t;

typedef enum {
  REC_ALT_EST = 1,
  REC_VEL_EST,
  REC_ALT_FIL,
  REC_ALT_LIM,
  REC_EVENT
} recorder_type_e;

typedef struct {
  uint8_t type;
  uint16_t ts;
  int16_t data;
} recorder_element_t;

bool recorder_init(recorder_t *rec, uint32_t size);
void recorder_deinit(recorder_t *rec);
void recorder_flush(recorder_t *rec);
bool recorder_record(recorder_t *rec, uint32_t ts, recorder_type_e type,
                     int16_t data);
uint32_t recorder_get_count(recorder_t *rec);
bool recorder_get_element(recorder_t *rec, uint32_t index,
                          recorder_element_t *element);
