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


osEventFlagsId_t buzzer_event_id;

BUZ_DEV BUZZER = {.timer = &htim4, .channel = TIM_CHANNEL_1, .arr = 4000, .start = 0, .started = 0, .volume = 100};
