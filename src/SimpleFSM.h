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

typedef void (*CallbackFunction)();
typedef bool (*GuardCondition)();

/////////////////////////////////////////////////////////////////

class SimpleFSM {
 public:
  SimpleFSM();
  SimpleFSM(State* initial_state);
  ~SimpleFSM();

  void add(Transition t[], int size);
  void add(TimedTransition t[], int size);

  void setInitialState(State* state);
  void setFinishedHandler(CallbackFunction f);
  void setTransitionHandler(CallbackFunction f);

  bool trigger(int event_id);
  void run(int interval = 1000, CallbackFunction tick_cb = NULL);
  void reset();

  bool isFinished() const;
  State* getState() const;
  bool isInState(State* state) const;
  State* getPreviousState() const;
  unsigned long lastTransitioned() const;
  String getDotDefinition();

 protected:
  int num_timed = 0;
  int num_standard = 0;
  Transition* transitions = NULL;
  TimedTransition* timed = NULL;

  bool is_initialized = false;
  bool is_finished = false;
  unsigned long last_run = 0;
  unsigned long last_transition = 0;

  State* inital_state = NULL;
  State* current_state = NULL;
  State* prev_state = NULL;
  CallbackFunction on_transition_cb = NULL;
  CallbackFunction finished_cb = NULL;

  String dot_definition = "";

  bool _initFSM();
  bool _transitionTo(AbstractTransition* transition);
  bool _changeToState(State* s, unsigned long now);

  void _addDotTransition(Transition& t);
  void _addDotTransition(TimedTransition& t);
  String _dot_transition(String from, String to, String label, String param);
  String _dot_inital_state();
  String _dot_header();
  String _dot_active_node();
};

/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////
