/////////////////////////////////////////////////////////////////

#pragma once
#ifndef TRANSITIONS_H
#define TRANSITIONS_H

/////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include "State.h"

/////////////////////////////////////////////////////////////////

typedef void (*CallbackFunction)();
typedef bool (*GuardCondition)();

/////////////////////////////////////////////////////////////////
// Constants for transition initialization
static constexpr int DEFAULT_EVENT_ID = 0;
static constexpr unsigned long DEFAULT_TIMER_VALUE = 0;

/////////////////////////////////////////////////////////////////
// abstract parent class for Transition and TimedTransition

class AbstractTransition {
  friend class SimpleFSM;

 public:
  AbstractTransition();
  // to make this class an interface
  virtual ~AbstractTransition(){};
  virtual int getID() const = 0;
  String getName() const;

  void setName(String name);
  void setOnRunHandler(CallbackFunction f);
  void setGuardCondition(GuardCondition f);

 protected:
  static int next_id;
  int id = 0;
  String name = "";

  State* from = NULL;
  State* to = NULL;
  String fromStateName = "";
  String toStateName = "";

  CallbackFunction on_run_cb = NULL;
  GuardCondition guard_cb = NULL;
};

/////////////////////////////////////////////////////////////////

class Transition : public AbstractTransition {
  friend class SimpleFSM;

 public:
  Transition();
  Transition(State* from, State* to, int event_id, CallbackFunction on_run = NULL, String name = "", GuardCondition guard = NULL);
  Transition(String from, String to, int event_id, CallbackFunction on_run = NULL, String name = "", GuardCondition guard = NULL);

  void setup(State* from, State* to, int event_id, CallbackFunction on_run = NULL, String name = "", GuardCondition guard = NULL);
  void setup(String from, String to, int event_id, CallbackFunction on_run = NULL, String name = "", GuardCondition guard = NULL);

  int getID() const;
  int getEventID() const;

 protected:
  int event_id;
};

/////////////////////////////////////////////////////////////////

class TimedTransition : public AbstractTransition {
  friend class SimpleFSM;

 public:
  TimedTransition();
  TimedTransition(State* from, State* to, int interval, CallbackFunction on_run = NULL, String name = "", GuardCondition guard = NULL);
  TimedTransition(String from, String to, int interval, CallbackFunction on_run = NULL, String name = "", GuardCondition guard = NULL);

  void setup(State* from, State* to, int interval, CallbackFunction on_run = NULL, String name = "", GuardCondition guard = NULL);
  void setup(String from, String to, int interval, CallbackFunction on_run = NULL, String name = "", GuardCondition guard = NULL);

  int getID() const;
  int getInterval() const;

  void reset();

 protected:
    unsigned long start;
    unsigned long interval;
};
/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////
