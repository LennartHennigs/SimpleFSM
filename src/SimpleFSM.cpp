/////////////////////////////////////////////////////////////////
#include "SimpleFSM.h"

#include "State.h"
#include "Transitions.h"
/////////////////////////////////////////////////////////////////

SimpleFSM::SimpleFSM() {
}

/////////////////////////////////////////////////////////////////

SimpleFSM::SimpleFSM(State* initial_state) {
  SimpleFSM();
  setInitialState(initial_state);
}

/////////////////////////////////////////////////////////////////

SimpleFSM::~SimpleFSM() {
  free(transitions);
  free(timed);
  transitions = NULL;
  timed = NULL;
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::reset() {
  is_initialized = 0;
  last_run = 0;
  setInitialState(inital_state);
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::setInitialState(State* state) {
  inital_state = state;
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::trigger(int event_id) {
  if (!is_initialized) _initFSM();
  // Find the transition with the current state and given event.
  for (int i = 0; i < num_standard; i++) {
    if (transitions[i].from == current_state && transitions[i].event_id == event_id) {
      return _transitionTo(&(transitions[i]));
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////

State* SimpleFSM::getPreviousState() const {
  return prev_state;
}

/////////////////////////////////////////////////////////////////

State* SimpleFSM::getState() const {
  return current_state;
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::isInState(State* t) const {
  return t == current_state;
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::setTransitionHandler(CallbackFunction f) {
  on_transition_cb = f;
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::add(Transition t[], int size) {
  if ( (transitions = (Transition *) malloc( size * sizeof( Transition ))) != NULL) {
    Transition* ptr;
    int i;
    num_standard = size;
    for ( ptr = transitions, i=0; i < size; ++i ) {
      *ptr++ = t[i];
      _addDotTransition(transitions[i]);
    } 
    num_standard = size;
  } else { /*  malloc error  */
    Serial.print( "Out of storage" );
    abort();
  }
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::addIncremental(Transition t[], int size) {
  if ( (transitions == NULL) || (num_standard == 0) ) {
    add(t, size);
  }
  else {
    if ( (transitions = (Transition *) realloc(transitions, (num_standard + size) * sizeof( Transition ))) != NULL) {    
      Transition* ptr;
      int i;
      for ( ptr = (transitions + num_standard), i=0; i < size; ++i ) {
        *ptr++ = t[i];
        _addDotTransition(transitions[(i+num_standard)]);
      }
      num_standard += size;
    }
    else { /* realloc error */
      Serial.print( "Out of storage" );
      abort();
    }
  }
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::add(TimedTransition t[], int size) {
  timed = t;
  num_timed = size;
  for (int i = 0; i < size; i++) {
    _addDotTransition(timed[i]);
  }
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::setFinishedHandler(CallbackFunction f) {
  finished_cb = f;
}

/////////////////////////////////////////////////////////////////

unsigned long SimpleFSM::lastTransitioned() const {
  return (last_transition == 0) ? 0 : (millis() - last_transition);
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::isFinished() const {
  return is_finished;
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::run(int interval /* = 1000 */, CallbackFunction tick_cb /* = NULL */) {
  unsigned long now = millis();
  // is the machine set up?
  if (!is_initialized) _initFSM();
  // are we ok?
  if (current_state == NULL) return;
  // is it time?
  if (now < last_run + interval) return;
  // are we done yet?
  if (is_finished) return;
  last_run = now;

  // go through the timed events
  for (int i = 0; i < num_timed; i++) {
    if (timed[i].from != current_state) continue;
    // start the transition timer
    if (timed[i].start == 0) {
      timed[i].start = now;
      continue;
    }
    // reached the interval?
    if (now - timed[i].start >= timed[i].interval) {
      if (_transitionTo(&timed[i])) {
        timed[i].start = 0;
        return;
      }
    }
  }
  // trigger the on_state event
  if (current_state->on_state != NULL) current_state->on_state();
  // trigger the regular tick event
  if (tick_cb != NULL) tick_cb();
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::_initFSM() {
  if (is_initialized) return false;
  is_initialized = true;
  if (inital_state == NULL) return false;
  return _changeToState(inital_state, millis());
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::_changeToState(State* s, unsigned long now) {
  if (s == NULL) return false;
  // set the new state
  prev_state = current_state;
  current_state = s;
  if (s->on_enter != NULL) s->on_enter();
  // save the time
  last_run = now;
  last_transition = now;
  // is this the end?
  if (s->is_final && finished_cb != NULL) finished_cb();
  if (s->is_final) is_finished = true;
  return true;
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::_transitionTo(AbstractTransition* transition) {
  // empty parameter?
  if (transition->to == NULL) return false;
  // can I pass the guard
  if (transition->guard_cb != NULL && !transition->guard_cb()) return false;
  // trigger events
  if (transition->from->on_exit != NULL) transition->from->on_exit();
  if (transition->on_run_cb != NULL) transition->on_run_cb();
  if (on_transition_cb != NULL) on_transition_cb();
  return _changeToState(transition->to, millis());
}

/////////////////////////////////////////////////////////////////

String SimpleFSM::getDotDefinition() {
  return "digraph G {\n" + _dot_header() + dot_definition + _dot_active_node() + _dot_inital_state() + "}\n";
}

/////////////////////////////////////////////////////////////////

String SimpleFSM::_dot_transition(String from, String to, String label, String param) {
  return "\t\"" + from + "\" -> \"" + to + "\"" + " [label=\"" + label + " (" + param + ")\"];\n";
}

/////////////////////////////////////////////////////////////////

String SimpleFSM::_dot_inital_state() {
  return inital_state ? "\t\"" + inital_state->getName() + "\" [style=filled fontcolor=white fillcolor=black];\n\n" : "";
}

/////////////////////////////////////////////////////////////////

String SimpleFSM::_dot_active_node() {
  return current_state ? "\t\"" + current_state->getName() + "\" [style=filled fontcolor=white];\n" : "";
}

/////////////////////////////////////////////////////////////////

String SimpleFSM::_dot_header() {
  return "\trankdir=LR; pad=0.5\n\tnode [shape=circle fixedsize=true width=1.5];\n";
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::_addDotTransition(Transition& t) {
  dot_definition = dot_definition + _dot_transition(t.from->getName(), t.to->getName(), t.getName(), "ID=" + String(t.event_id));
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::_addDotTransition(TimedTransition& t) {
  dot_definition = dot_definition + _dot_transition(t.from->getName(), t.to->getName(), t.getName(), String(t.getInterval()) + "ms");
}

/////////////////////////////////////////////////////////////////