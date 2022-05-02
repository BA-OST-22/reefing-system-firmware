/*
 * Reefing System Bachelor Thesis Software
 * Copyright (C) 2022 Institute for Microelectronics and Embedded Systems OST
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

#include "recorder/recorder.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief  Initialize the recorder
 *
 * @param  recorder: recorder structure
 * @param  size: size of the buffer
 * @retval None
 */
bool recorder_init(recorder_t *rec, uint32_t size) {
  rec->data = (uint8_t *)malloc(size);

  if (rec->data == NULL)
    return false;

  rec->index = 0;
  rec->size = size;
  rec->element_count = 0;
  return true;
}

/**
 * @brief  Deinitialize the recorder
 *
 * @param  rec: recorder structure
 * @retval None
 */
void recorder_deinit(recorder_t *rec) {
  rec->index = 0;
  rec->size = 0;
  rec->element_count = 0;
  if (rec->data == NULL)
    return;
  free(rec->data);
}

/**
 * @brief flush all data stored in the recorder
 *
 * @param rec: recorder structure
 * @retval None
 */
void recorder_flush(recorder_t *rec) {
  rec->first_ts = 0;
  rec->index = 0;
  rec->element_count = 0;
}

/**
 * @brief  Add a new element to the recorder, each element has a maximum size of
 * int16
 *
 * @param  rec: recorder structure
 * @param  ts: timestamp of the element
 * @param  type: type of the element
 * @param  data: data of the element
 * @retval None
 */
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

/**
 * @brief Get the number of elements stored in the recorder
 *
 * @param rec: recorder structure
 * @return uint32_t: number of elements
 */
uint32_t recorder_get_count(recorder_t *rec) { return rec->element_count; }

/**
 * @brief Read an element from the recorder
 *
 * @param rec: recorder structure
 * @param index: index of the element to read
 * @param element: pointer to where to store the element
 * @return true: element read, false: element not read
 */
bool recorder_get_element(recorder_t *rec, uint32_t index,
                          recorder_element_t *element) {
  if ((index == 0) || (index > rec->element_count))
    return false;
  uint32_t start = (index - 1) * sizeof(recorder_element_t);
  memcpy((uint8_t *)element, &rec->data[start], sizeof(recorder_element_t));
  return true;
}
