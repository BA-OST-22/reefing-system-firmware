/*
 * task_buzzer.c
 *
 *  Created on: 7 Apr 2022
 *      Author: Luca
 */

#include "cmsis_os.h"
#include "task_buzzer.h"
#include "config/globals.h"
#include "util/log.h"

#include "drivers/buzzer.h"

#define BUZZER_COMMAND_MAX_LENGTH 9

#define BUZZER_SHORT_BEEP  40
#define BUZZER_LONG_BEEP   100
#define BUZZER_SHORT_PAUSE 100
#define BUZZER_LONG_PAUSE  1000

static BUZ_DEV BUZZER = {.timer = &htim4, .channel = TIM_CHANNEL_1, .arr = 4000, .start = 0, .started = 0, .volume = 100};

static const uint32_t pitch_lookup[8] = {
	2217,  // C# 	A
	2349,  // D 	B
    2489,  // D# 	C
    2637,  // E 	D
    2793,  // F 	E
    2959,  // F# 	F
    3135,  // G 	G
	1200,  // Error H
};

static const uint8_t nr_buz[6] = {
		0, 1, 1, 4, 3, 3
};

static const char beep_codes[7][BUZZER_COMMAND_MAX_LENGTH] = {
    " ",
    "g",   // ok
	"h",   // nok
	"DBFG" // bootup
    "ace",  // ready
    "eca",  // not ready
	"adc",
};

void task_buzzer(void *argument) {
	beeps_e id;

	buzzer_set_volume(&BUZZER, 20);
	buzzer_set_freq(&BUZZER, 2500);

	while (1) {
		id = osEventFlagsWait(buzzer_event_id, 0xFF, osFlagsWaitAny, osWaitForever);
		osEventFlagsClear(buzzer_event_id, id);
		uint32_t duration = 0;
		for(int i = 0; i < nr_buz[id]; i++){
			char pitch = beep_codes[id][i];
			if (pitch >= 'A' && pitch <= 'H') {
				buzzer_set_freq(&BUZZER, pitch_lookup[pitch - 'A']);
				duration = BUZZER_LONG_BEEP;
			} else if (pitch >= 'a' && pitch <= 'h') {
				buzzer_set_freq(&BUZZER, pitch_lookup[pitch - 'a']);
				duration = BUZZER_SHORT_BEEP;
			}
			buzzer_start(&BUZZER);
			osDelay(duration);
			buzzer_stop(&BUZZER);
			osDelay(BUZZER_SHORT_PAUSE);
		}

		// Wait at least 1s before buzzing again
		osDelay(BUZZER_LONG_PAUSE);
	}
}
