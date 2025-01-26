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
  void add(State s[], int size);

  void setInitialState(State* state);
  void setFinishedHandler(CallbackFunction f);
  void setTransitionHandler(CallbackFunction f);

  bool trigger(int event_id);
  void run(int interval = 1000, CallbackFunction tick_cb = NULL);
  void reset();

  int getTransitionCount() const;
  int getTimedTransitionCount() const;
  int getStateCount() const;
  
  bool isFinished() const;
  State* getState() const;
  bool isInState(State* state) const;
  State* getPreviousState() const;
  AbstractTransition* getLastTransition() const;
  unsigned long lastTransitioned() const;
  String getDotDefinition(bool showActive = true);

 protected:
  bool isSetupOK() const;

  int num_timed = 0;
  int num_standard = 0;
  int num_states = 0;
  Transition* transitions = NULL;
  TimedTransition* timed = NULL;
  State* states = NULL;

  bool is_initialized = false;
  bool is_finished = false;
  unsigned long last_run = 0;
  unsigned long last_transition = 0;
  AbstractTransition* last_transition_ptr = NULL;

  State* inital_state = NULL;
  State* current_state = NULL;
  State* prev_state = NULL;
  CallbackFunction on_transition_cb = NULL;
  CallbackFunction finished_cb = NULL;

  String dot_definition = "";

  bool isDuplicate(const TimedTransition& transition, const TimedTransition* transitionArray, int arraySize) const;
  bool isDuplicate(const Transition& transition, const Transition* transitionArray, int arraySize) const;

  bool isTimeForRun(unsigned long now, int interval);
  void handleTimedEvents(unsigned long now);

  State* getStateByName(String name);

  bool initFSM();
  bool transitionTo(AbstractTransition* transition);
  bool changeToState(State* s, unsigned long now);
  void checkAndInitializeTransitions();

  String getDOTHeader();
  void addDOTTransition(Transition& t);
  void addDOTTransition(TimedTransition& t);
  String getDOTTransition(String from, String to, String label, String param);
  String getDOTInitalState();
  String getDOTActiveNode();

  bool isStateInArray(State* state, State* stateArray[], int arraySize);

};

/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////
