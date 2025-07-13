/////////////////////////////////////////////////////////////////
// FSMTransitionUtils.cpp - Utility functions for SimpleFSM
/////////////////////////////////////////////////////////////////
#include "FSMTransitionUtils.h"

/////////////////////////////////////////////////////////////////
void createManyToOneTransitions(
    State* sources[], int num_sources,
    State* target,
    int event_id,
    Transition* out_array,
    CallbackFunction on_run,
    String name,
    GuardCondition guard
) {
    for (int i = 0; i < num_sources; ++i) {
        out_array[i] = Transition(sources[i], target, event_id, on_run, name, guard);
    }
}

/////////////////////////////////////////////////////////////////
void createManyToOneTimedTransitions(
    State* sources[], int num_sources,
    State* target,
    int interval,
    TimedTransition* out_array,
    CallbackFunction on_run,
    String name,
    GuardCondition guard
) {
    for (int i = 0; i < num_sources; ++i) {
        out_array[i] = TimedTransition(sources[i], target, interval, on_run, name, guard);
    }
}
/////////////////////////////////////////////////////////////////
