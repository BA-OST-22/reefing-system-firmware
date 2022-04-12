
#pragma once

typedef enum {
	WAKEUP_BUTTON = 1,
	WAKEUP_ACCEL,
	WAKEUP_BOTH,
} wakeup_e;

void go_to_sleep(wakeup_e source);
void wake_up();
