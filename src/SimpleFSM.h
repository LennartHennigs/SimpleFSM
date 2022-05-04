/////////////////////////////////////////////////////////////////

#pragma once
#ifndef SIMPLE_FSM_H
#define SIMPLE_FSM_H

/////////////////////////////////////////////////////////////////

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP8266)
  #include <functional>
#endif
#include "Arduino.h"

#include "State.h"
#include "Transitions.h"

/////////////////////////////////////////////////////////////////

typedef void (*CallbackFunction) ();
typedef bool (*GuardCondition) ();

/////////////////////////////////////////////////////////////////

class SimpleFSM {
  public:
    SimpleFSM();
    SimpleFSM(State* initial_state);
    ~SimpleFSM();

    bool add(Transition t[], int size);
    bool add(TimedTransition t[], int size);

    void setInitialState(State* state);
    void setFinishedHandler(CallbackFunction f);
    void setTransitionHandler(CallbackFunction f);

    bool trigger(int event_id);
    void run(int interval = 1000, CallbackFunction tick_cb = NULL);
    void reset();

    State* getState() const;
    State* getPreviousState() const;
    bool isInState(State* state) const;
    int getTimeSinceTransition() const;

  protected:
    int num_timed = 0;
    int num_standard = 0;
    Transition* transitions = NULL;
    TimedTransition* timed_transitions = NULL;

    bool is_initialized = false;
    String dot_definition = "";
    State* inital_state = NULL;
    State* current_state = NULL;
    State* prev_state = NULL;
    CallbackFunction on_transition = NULL;
    CallbackFunction finished_cb = NULL;
    unsigned long last_run = 0;
    unsigned long last_transition = -1;

    bool _transitionTo(AbstractTransition* transition);
    bool _changeToState(State* s, unsigned long now);
    bool _initFSM();
};

/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////
