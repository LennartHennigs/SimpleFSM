/////////////////////////////////////////////////////////////////
#include "SimpleFSM.h"
/////////////////////////////////////////////////////////////////

SimpleFSM::SimpleFSM() : 
  transitions(NULL),
  timed(NULL),
  num_standard(0),
  num_timed(0),
  last_run(0),
  is_initialized(false),
  on_transition_cb(NULL),
  finished_cb(NULL),
  inital_state(NULL),
  dot_definition(""),
  current_state(NULL)
{}

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

bool SimpleFSM::add(Transition t[], int size) {
  transitions = (Transition*) realloc(transitions, (num_standard + size) * sizeof(Transition));
  for (int i=0; i < size; i++) {
    transitions[num_standard + i] = t[i];
    _addDotTransition(t[i]);
  }
  num_standard = num_standard + size;
  return true;
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::add(TimedTransition t[], int size) {
  timed = (TimedTransition*) realloc(timed, (num_timed + size) * sizeof(TimedTransition));
  for (int i=0; i < size; i++) {
    timed[num_timed + i] = t[i];
    _addDotTransition(t[i]);
  }
  num_timed = num_timed + size;
  return true;

}

/////////////////////////////////////////////////////////////////

void SimpleFSM::setFinishedHandler(CallbackFunction f) {
  finished_cb = f;
}

/////////////////////////////////////////////////////////////////

int SimpleFSM::lastTransitionedAt() const {
  return (last_transition == -1) ? -1 : millis() - last_transition;
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
  if (current_state != NULL) {
    if (now >= last_run + interval) {
      last_run = now;
      // are we done yet?
      if (!is_finished) {
        // trigger the on_state event
        if (current_state->on_state != NULL) current_state->on_state();

        // go through the timed events
        for (int i = 0; i < num_timed; i++) {
          // am I in the right state
          if (timed[i].from == current_state) {
            // need to reset timer of transition?
            if (timed[i].start == 0) {
              timed[i].start = now;
            // reached the interval?
            } else if (now - timed[i].start >= timed[i].interval) {
              if (_transitionTo(&timed[i])) {
                timed[i].start = 0;
                return;
              }
            }
          }
        }
        // trigger the regular tick event
        if (tick_cb != NULL) tick_cb();
      }
    }
  }
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::_initFSM() {
  unsigned long now = millis();
  // do we need to do this?
  if (!is_initialized) {
    is_initialized = true;
    // is the inital state set?
    if (inital_state != NULL) {
      return _changeToState(inital_state, now);
    } else {
      return false;
    }
  }
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::_changeToState(State* s, unsigned long now) {
  if (s != NULL) {
    prev_state = current_state;
    current_state = s;
    if (s->on_enter != NULL) s->on_enter();
    last_run = now;
    // to make it accessible within on_enter
    last_transition = now;
    // is this the end?
    if (s->is_final && finished_cb != NULL) finished_cb();
    if (s->is_final) is_finished = true;
    return true;
  } else {
    return false;
  }
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::_transitionTo(AbstractTransition* transition) {
  unsigned long now = millis();
  if (transition->to == NULL) {
    return false;
  } 
  // can I pass the guard
  if (transition->guard_cb != NULL && !transition->guard_cb()) {
    return false;
  }
  if (transition->from->on_exit != NULL) transition->from->on_exit();      
  if (transition->on_run_cb != NULL) transition->on_run_cb();
  if (on_transition_cb != NULL) on_transition_cb();
  
  return _changeToState(transition->to, now);
}

/////////////////////////////////////////////////////////////////

String SimpleFSM::getDotDefinition() {
  return "digraph G {\n" + _dot_header() + dot_definition + _dot_inital_state() + _dot_active_node() + "}\n";
}

/////////////////////////////////////////////////////////////////

String SimpleFSM::_dot_transition(String from, String to, String label, String param) {
  return "\t\"" + from + "\" -> \"" + to + "\"" +  " [label=\"" + label + " (" + param + ")\"];\n";
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
