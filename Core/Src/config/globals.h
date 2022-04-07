/*
 * globals.h
 *
 *  Created on: Apr 5, 2022
 *      Author: Luca
 */

#pragma once

#include "util/fifo.h"
#include "cmsis_os2.h"
#include "drivers/buzzer.h"


/* Sampling rate for all time dependent operations */
#define SAMPLING_FREQ 100 // Hz

/* USB fifos */

#define USB_OUTPUT_BUFFER_SIZE 512
#define USB_INPUT_BUFFER_SIZE 512

extern fifo_t usb_input_fifo;
extern fifo_t usb_output_fifo;

extern uint8_t usb_fifo_out_buffer[USB_OUTPUT_BUFFER_SIZE];
extern uint8_t usb_fifo_in_buffer[USB_INPUT_BUFFER_SIZE];


typedef enum{
	NO_BEEP = 0,
	BEEP_OK,
	BEEP_NOK,
	BEEP_BOOTUP,
	BEEP_READY,
	BEEP_NOT_READY,
}beeps_t;

extern osEventFlagsId_t buzzer_event_id;

extern BUZ_DEV BUZZER;
