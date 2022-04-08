#pragma once

/* Transition Deep Sleep -> Idle */
void state_transition_deepsleep_idle();

/* Transition Idle -> Deep Sleep */
void state_transition_idle_deepsleep();

/* Transition Idle -> Ready */
void state_transition_idle_ready();

/* Transition Ready -> Idle */
void state_transition_ready_idle();

/* Transition Ready -> Ready Sleep */
void state_transition_ready_readysleep();

/* Transition Ready Sleep -> Ready */
void state_transition_readysleep_ready();

/* Transition Ready -> Ascent */
void state_transition_ready_ascent();

/* Transition Ascent -> Descent*/
void state_transition_ascent_descent();

/* Transition Descent -> Deployment */
void state_transition_descent_deplyment();

/* Transition Deployment -> Recovery */
void state_transition_deployment_recovery();

/* Transition Recovery -> Deep Sleep */
void state_transition_recovery_deepsleep();
