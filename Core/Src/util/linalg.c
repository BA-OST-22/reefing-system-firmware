/*
 * linalg.c
 *
 *  Created on: 16 Apr 2022
 *      Author: Luca
 */

#include "util/linalg.h"
#include <string.h>

/*
 * C = A*B
 * A [row_a*column_a]
 * B [column_a*column_b]
 * C [row_a*column_b]
 */
void mul(float A[], float B[], float C[], uint16_t row_a, uint16_t column_a, uint16_t column_b) {

	// Data matrix
	float* data_a;
	float* data_b;

	for (uint16_t i = 0; i < row_a; i++) {
		// Then we go through every column of b
		for (uint16_t j = 0; j < column_b; j++) {
			data_a = &A[i * column_a];
			data_b = &B[j];

			*C = 0; // Reset
			// And we multiply rows from a with columns of b
			for (uint16_t k = 0; k < column_a; k++) {
				*C += *data_a * *data_b;
				data_a++;
				data_b += column_b;
			}
			C++; // ;)
		}
	}
}


/*
 * Turn A into transponse A^T
 */
void tran(float A[], uint16_t row, uint16_t column) {

	float B[row*column];
	float* transpose;
	float* ptr_A = A;

	for (uint16_t i = 0; i < row; i++) {
		transpose = &B[i];
		for (uint16_t j = 0; j < column; j++) {
			*transpose = *ptr_A;
			ptr_A++;
			transpose += row;
		}
	}

	// Copy!
	memcpy(A, B, row*column*sizeof(float));
}

/*
 * C = A+B
 */
void add(float A[], float B[], float C[], uint16_t size) {

	// Data matrix
	for (uint16_t i = 0; i < size; i++) {
		C[i] = A[i] + B[i];
	}
}

/*
 * C = A-B
 */
void sub(float A[], float B[], float C[], uint16_t size) {

	// Data matrix

	for (uint16_t i = 0; i < size; i++) {
		C[i] = A[i] - B[i];
	}
}

