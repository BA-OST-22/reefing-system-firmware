#pragma once

#include "target/target.h"

void mul(float A[], float B[], float C[], uint16_t row_a, uint16_t column_a, uint16_t column_b);
void tran(float A[], uint16_t row, uint16_t column);
void add(float A[], float B[], float C[], uint16_t size);
void sub(float A[], float B[], float C[], uint16_t size);

