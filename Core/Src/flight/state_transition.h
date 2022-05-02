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
void state_transition_descent_deployment();

/* Transition Deployment -> Recovery */
void state_transition_deployment_recovery();

/* Transition Recovery -> Idle */
void state_transition_recovery_idle();
