/////////////////////////////////////////////////////////////////
// FSMTransitionUtils.h - Utility functions for SimpleFSM
/////////////////////////////////////////////////////////////////
#pragma once

#include "State.h"
#include "Transitions.h"

/////////////////////////////////////////////////////////////////
// Helper: Create many-to-one regular transitions
void createManyToOneTransitions(
    State* sources[], int num_sources,
    State* target,
    int event_id,
    Transition* out_array,
    CallbackFunction on_run = NULL,
    String name = "",
    GuardCondition guard = NULL
);

/////////////////////////////////////////////////////////////////
// Helper: Create many-to-one timed transitions
void createManyToOneTimedTransitions(
    State* sources[], int num_sources,
    State* target,
    int interval,
    TimedTransition* out_array,
    CallbackFunction on_run = NULL,
    String name = "",
    GuardCondition guard = NULL
);
/////////////////////////////////////////////////////////////////
