/*
 * recorder.c
 *
 *  Created on: 16 Apr 2022
 *      Author: Luca
 */
#include "recorder/recorder.h"
#include <stdlib.h>
#include <string.h>

bool recorder_init(recorder_t *rec, uint32_t size) {
  rec->data = (uint8_t *)malloc(size);

  if (rec->data == NULL)
    return false;

  rec->index = 0;
  rec->size = size;
  rec->element_count = 0;
  return true;
}

void recorder_deinit(recorder_t *rec) {
  rec->index = 0;
  rec->size = 0;
  rec->element_count = 0;
  if (rec->data == NULL)
    return;
  free(rec->data);
}

void recorder_flush(recorder_t *rec) {
  rec->first_ts = 0;
  rec->index = 0;
  rec->element_count = 0;
}

bool recorder_record(recorder_t *rec, uint32_t ts, recorder_type_e type,
                     int16_t data) {

  if (rec->data == NULL)
    return false;

  if ((rec->index + sizeof(recorder_element_t)) > rec->size)
    return false;

  if (rec->index == 0)
    rec->first_ts = ts;

  recorder_element_t element;

  element.type = (uint8_t)type;
  element.ts = ts - rec->first_ts;
  element.data = data;

  memcpy(&rec->data[rec->index], (uint8_t *)&element,
         sizeof(recorder_element_t));
  rec->index += sizeof(recorder_element_t);
  rec->element_count++;

  return true;
}

uint32_t recorder_get_count(recorder_t *rec) { return rec->element_count; }

bool recorder_get_element(recorder_t *rec, uint32_t index,
                          recorder_element_t *element) {
  if ((index == 0) || (index > rec->element_count))
    return false;
  uint32_t start = (index - 1) * sizeof(recorder_element_t);
  memcpy((uint8_t *)element, &rec->data[start], sizeof(recorder_element_t));
  return true;
}
