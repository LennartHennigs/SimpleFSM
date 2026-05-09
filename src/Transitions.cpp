/////////////////////////////////////////////////////////////////
#include "Transitions.h"
/////////////////////////////////////////////////////////////////

int AbstractTransition::next_id = 0;

/////////////////////////////////////////////////////////////////

AbstractTransition::AbstractTransition() {
  id = next_id;
  next_id++;
}

/////////////////////////////////////////////////////////////////

void AbstractTransition::setGuardCondition(GuardCondition f) {
  guard_cb = f;
}

/////////////////////////////////////////////////////////////////

String AbstractTransition::getName() const {
  return name;
}

/////////////////////////////////////////////////////////////////

void AbstractTransition::setName(String name) {
  this->name = name;
}

/////////////////////////////////////////////////////////////////

void AbstractTransition::setOnRunHandler(CallbackFunction f) {
  on_run_cb = f;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// Transition
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

int Transition::getEventID() const {
  return event_id;
}

/////////////////////////////////////////////////////////////////

int Transition::getID() const {
  return id;
}

/////////////////////////////////////////////////////////////////

Transition::Transition() : event_id(DEFAULT_EVENT_ID) {}

/////////////////////////////////////////////////////////////////

Transition::Transition(State* from, State* to, int event_id, CallbackFunction on_run /* = NULL */, String name /* = "" */, GuardCondition guard /* = NULL */) {
  setup(from, to, event_id, on_run, name, guard);
}

/////////////////////////////////////////////////////////////////

TimedTransition::TimedTransition(String from, String to, int interval, CallbackFunction on_run /* = NULL */, String name /* = "" */, GuardCondition guard /*= NULL */) {
  setup(from, to, interval, on_run, name, guard);
}

/////////////////////////////////////////////////////////////////

Transition::Transition(String from, String to, int event_id, CallbackFunction on_run /*= NULL */, String name /* = "" */, GuardCondition guard /*= NULL */) {
  setup(from, to, event_id, on_run, name, guard);
}

/////////////////////////////////////////////////////////////////

void Transition::setup(State* from, State* to, int event_id, CallbackFunction on_run /* = NULL */, String name /* = "" */, GuardCondition guard /* = NULL */) {
  this->from = from;
  this->to = to;
  this->event_id = event_id;
  this->on_run_cb = on_run;
  this->name = name;
  this->guard_cb = guard;
}

/////////////////////////////////////////////////////////////////

void Transition::setup(String from, String to, int event_id, CallbackFunction on_run /* = NULL */, String name /* = "" */, GuardCondition guard /* = NULL */) {
  this->fromStateName = from;
  this->toStateName = to;
  this->event_id = event_id;
  this->on_run_cb = on_run;
  this->name = name;
  this->guard_cb = guard;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// TimedTransition
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

TimedTransition::TimedTransition() : start(DEFAULT_TIMER_VALUE), interval(DEFAULT_TIMER_VALUE) {}

/////////////////////////////////////////////////////////////////

TimedTransition::TimedTransition(State* from, State* to, int interval, CallbackFunction on_run /* = NULL */, String name /* = "" */, GuardCondition guard /* = NULL */) : TimedTransition() {
  setup(from, to, interval, on_run, name, guard);
}

/////////////////////////////////////////////////////////////////

void TimedTransition::setup(State* from, State* to, int interval, CallbackFunction on_run /* = NULL */, String name /* = "" */, GuardCondition guard /* = NULL */) {
  this->from = from;
  this->to = to;
  this->interval = interval;
  this->on_run_cb = on_run;
  this->name = name;
  this->guard_cb = guard;
}

/////////////////////////////////////////////////////////////////

void TimedTransition::setup(String from, String to, int interval, CallbackFunction on_run /* = NULL */, String name /* = "" */, GuardCondition guard /* = NULL */) {
  this->fromStateName = from;
  this->toStateName = to;
  this->interval = interval;
  this->on_run_cb = on_run;
  this->name = name;
  this->guard_cb = guard;
}

/////////////////////////////////////////////////////////////////

int TimedTransition::getInterval() const {
  return interval;
}

/////////////////////////////////////////////////////////////////

int TimedTransition::getID() const {
  return id;
}

/////////////////////////////////////////////////////////////////

void TimedTransition::reset() {
  start = DEFAULT_TIMER_VALUE;
}

/////////////////////////////////////////////////////////////////