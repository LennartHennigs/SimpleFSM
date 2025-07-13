/////////////////////////////////////////////////////////////////
// FSMUtils.h - Utility functions for SimpleFSM
/////////////////////////////////////////////////////////////////
#pragma once

#include "State.h"
#include "Transitions.h"

/////////////////////////////////////////////////////////////////
// Helper class for utility functions
class FSMUtils {
public:
    // Create many-to-one regular transitions
    static void createManyToOneTransitions(
        State* sources[], int num_sources,
        State* target,
        int event_id,
        Transition* out_array,
        CallbackFunction on_run = NULL,
        String name = "",
        GuardCondition guard = NULL
    );

    // Create many-to-one timed transitions
    static void createManyToOneTimedTransitions(
        State* sources[], int num_sources,
        State* target,
        int interval,
        TimedTransition* out_array,
        CallbackFunction on_run = NULL,
        String name = "",
        GuardCondition guard = NULL
    );
};
/////////////////////////////////////////////////////////////////
