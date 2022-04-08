
#include "fsm.h"
#include "target/target.h"
#include "flight/state_transition.h"
#include "util/log.h"

static void check_idle_state(fsm_t* fsm);
static void check_deepsleep_state(fsm_t* fsm);
static void check_ready_state(fsm_t* fsm);
static void check_readysleep_state(fsm_t* fsm);
static void check_ascent_state(fsm_t* fsm);
static void check_descent_state(fsm_t* fsm);
static void check_deplyoment_state(fsm_t* fsm);
static void check_recovery_state(fsm_t* fsm);

const char* const state_name[] =
{	"INVALID",
	"IDLE",
	"DEEP_SLEEP",
	"READY",
	"READY_SLEEP",
	"ASCENT",
	"DESCENT",
	"DEPLOYMENT",
	"RECOVERY",
};

void update_fsm(fsm_t* fsm){
	fsm_state_e old_state = fsm->flight_state;
	switch(fsm->flight_state){
	case IDLE:
		check_idle_state(fsm);
		break;
	case DEEP_SLEEP:
		check_deepsleep_state(fsm);
		break;
	case READY:
		check_ready_state(fsm);
		break;
	case READY_SLEEP:
		check_readysleep_state(fsm);
		break;
	case ASCENT:
		check_ascent_state(fsm);
		break;
	case DESCENT:
		check_descent_state(fsm);
		break;
	case DEPLOYMENT:
		check_deplyoment_state(fsm);
		break;
	case RECOVERY:
		check_recovery_state(fsm);
		break;
	default:
		break;
	}

	if(old_state != fsm->flight_state){
		log_warn("State Transition %s to %s", state_name[old_state], state_name[fsm->flight_state]);
	}
}

static void check_idle_state(fsm_t* fsm){

	/* When button is pressed */
	if(HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0){
		if(fsm->memory[1] == 0){
			fsm->memory[0]++;
		} else {
			fsm->memory[2]++;
		}
	}
	/* When button is not pressed */
	else {
		/* Short button press */
		if(fsm->memory[0] > SHORT_BUTTON_PRESS){
			fsm->memory[1]++; // Count timeout between button press
		}
		/* Button bounce or no press */
		else{
			fsm->memory[0] = 0;
			fsm->memory[2] = 0;
		}
	}

	/* Long button press */
	if (fsm->memory[0] > LONG_BUTTON_PRESS){
		/* State Transition to READY */
		fsm->memory[0] = 0;
		fsm->memory[1] = 0;
		fsm->memory[2] = 0;
		fsm->flight_state = READY;
		state_transition_idle_ready();
	}

	/* Long press after short press */
	if(fsm->memory[2] > LONG_BUTTON_PRESS){
		/* State Transition to DEEP_SLEEP */
		fsm->memory[0] = 0;
		fsm->memory[1] = 0;
		fsm->memory[2] = 0;
		fsm->flight_state = DEEP_SLEEP;
		state_transition_idle_deepsleep();
	}

	/* Timeout after short press */
	if(fsm->memory[1] > TIMEOUT_BETWEEN_BUTTON_PRESS){
		fsm->memory[0] = 0;
		fsm->memory[1] = 0;
		fsm->memory[2] = 0;
	}
}

static void check_deepsleep_state(fsm_t* fsm){

	/* When button is pressed */
	if(HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0){
		if(fsm->memory[1] == 0){
			fsm->memory[0]++;
		} else {
			fsm->memory[2]++;
		}
	}
	/* When button is not pressed */
	else {
		fsm->memory[1]++; // Count timeout between button press
	}

	/* Long press after short press */
	if(fsm->memory[2] > LONG_BUTTON_PRESS){
		/* State Transition to IDLE */
		fsm->memory[0] = 0;
		fsm->memory[1] = 0;
		fsm->memory[2] = 0;
		fsm->flight_state = IDLE;
		state_transition_deepsleep_idle();
	}

	/* Timeout after short press */
	if(fsm->memory[1] > TIMEOUT_BETWEEN_BUTTON_PRESS){
		fsm->memory[0] = 0;
		fsm->memory[1] = 0;
		fsm->memory[2] = 0;
	}
}

static void check_ready_state(fsm_t* fsm){

	/* When button is pressed */
	if(HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0){
		fsm->memory[0]++;
	}
	/* When button is not pressed */
	else {
		fsm->memory[0] = 0; // Count timeout between button press
	}

	/* Long button press */
	if (fsm->memory[0] > LONG_BUTTON_PRESS){
		/* State Transition to IDLE */
		fsm->memory[0] = 0;
		fsm->memory[1] = 0;
		fsm->memory[2] = 0;
		fsm->flight_state = IDLE;
		state_transition_ready_idle();
	}
}

static void check_readysleep_state(fsm_t* fsm){

}

static void check_ascent_state(fsm_t* fsm){

}

static void check_descent_state(fsm_t* fsm){

}

static void check_deplyoment_state(fsm_t* fsm){

}

static void check_recovery_state(fsm_t* fsm){

}
