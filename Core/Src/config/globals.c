/*
 * globals.c
 *
 *  Created on: Apr 5, 2022
 *      Author: Luca
 */


#include "globals.h"


/* USB buffers and fifos */

fifo_t usb_input_fifo;
fifo_t usb_output_fifo;

uint8_t usb_fifo_out_buffer[USB_OUTPUT_BUFFER_SIZE];
uint8_t usb_fifo_in_buffer[USB_INPUT_BUFFER_SIZE];
