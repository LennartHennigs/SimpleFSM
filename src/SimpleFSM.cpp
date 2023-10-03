/////////////////////////////////////////////////////////////////
#include "SimpleFSM.h"

#include "State.h"
#include "Transitions.h"
/////////////////////////////////////////////////////////////////

SimpleFSM::SimpleFSM() {
}

/////////////////////////////////////////////////////////////////
/*
 * Constructor.
 */

SimpleFSM::SimpleFSM(State* initial_state) {
  SimpleFSM();
  setInitialState(initial_state);
}

/////////////////////////////////////////////////////////////////
/*
 * Destructor.
 */

SimpleFSM::~SimpleFSM() {
  transitions = NULL;
  timed = NULL;
}

/////////////////////////////////////////////////////////////////
/*
 * Get the number of transitions.
 */

int SimpleFSM::getTransitionCount() const {
  return num_standard;
}

/////////////////////////////////////////////////////////////////
/*
 * Get the number of timed transitions.
 */

int SimpleFSM::getTimedTransitionCount() const {
  return num_timed;
}

/////////////////////////////////////////////////////////////////
/*
 * Reset the FSM to its initial state.
 */

void SimpleFSM::reset() {
  is_initialized = false;
  is_finished = false;
  last_run = 0;
  last_transition = 0;
  setInitialState(inital_state);
  current_state = NULL;
  prev_state = NULL;

  for (int i = 0; i < num_timed; i++) {
    timed[i].reset();
  }
}

/////////////////////////////////////////////////////////////////
/*
 * Set the initial state.
 */

void SimpleFSM::setInitialState(State* state) {
  inital_state = state;
}

/////////////////////////////////////////////////////////////////
/*
 * Trigger an event.
 */

bool SimpleFSM::trigger(int event_id) {
  if (!is_initialized) _initFSM();
  // Find the transition with the current state and given event
  for (int i = 0; i < num_standard; i++) {
    if (transitions[i].from == current_state && transitions[i].event_id == event_id) {
      return _transitionTo(&(transitions[i]));
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////
/*
 * Get the previous state.
 */ 

State* SimpleFSM::getPreviousState() const {
  return prev_state;
}

/////////////////////////////////////////////////////////////////
/*
 * Get the current state.
 */

State* SimpleFSM::getState() const {
  return current_state;
}

/////////////////////////////////////////////////////////////////
/*
 * Check if the FSM is in a given state.
 */

bool SimpleFSM::isInState(State* t) const {
  return t == current_state;
}

/////////////////////////////////////////////////////////////////
/*
 * Sets the transition handler.
 */

void SimpleFSM::setTransitionHandler(CallbackFunction f) {
  on_transition_cb = f;
}

/////////////////////////////////////////////////////////////////
/* 
 * Add transitions to the FSM.
 * 
 * @param t[] An array of transitions.
 * @param size The size of the array.  
 */

void SimpleFSM::add(Transition newTransitions[], int size) {
  // Allocate or expand storage for transitions
  Transition* temp = new Transition[num_standard + size];
  if (transitions != NULL) {
    memcpy(temp, transitions, num_standard * sizeof(Transition));
    delete[] transitions;
  }
  transitions = temp;
  // Check if memory allocation was successful
  if (transitions == NULL) {
    Serial.print("Out of storage");
    abort();
  }
  // Add new transitions, avoiding duplicates
  for (int i = 0; i < size; ++i) {
    if (!_isDuplicate(newTransitions[i], transitions, num_standard) &&
        !_isDuplicate(newTransitions[i], newTransitions, i)) {
      transitions[num_standard] = newTransitions[i];
      _addDotTransition(transitions[num_standard]);
      num_standard++;
    }
  }
}

/////////////////////////////////////////////////////////////////
/*
  * Add timed transitions to the FSM.
  * 
  * @param t[] An array of timed transitions.
  * @param size The size of the array.  
  */

void SimpleFSM::add(TimedTransition newTransitions[], int size) {
  // Allocate memory or expand existing storage
  TimedTransition* temp = new TimedTransition[num_timed + size];
  if (timed != NULL) {
    memcpy(temp, timed, num_timed * sizeof(TimedTransition));
    delete[] timed;
  }
  timed = temp;
  // Check memory allocation
  if (timed == NULL) {
    Serial.print("Out of storage");
    abort();
  }
  // Add new transitions while avoiding duplicates
  for (int i = 0; i < size; ++i) {
    if (!_isDuplicate(newTransitions[i], timed, num_timed) && 
        !_isDuplicate(newTransitions[i], newTransitions, i)) {
      timed[num_timed] = newTransitions[i];
      _addDotTransition(timed[num_timed]);
      num_timed++;
    }
  }
}

/////////////////////////////////////////////////////////////////
/*
 * Check if a timed transition is a duplicate.
 */
 
bool SimpleFSM::_isDuplicate(const TimedTransition& transition, const TimedTransition* transitionArray, int arraySize) const {
  for (int i = 0; i < arraySize; ++i) {
    if (transitionArray[i].from == transition.from &&
        transitionArray[i].to == transition.to &&
        transitionArray[i].interval == transition.interval) {
      return true;
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////
/*
 * Check if a transition is a duplicate.
 */

bool SimpleFSM::_isDuplicate(const Transition& transition, const Transition* transitionArray, int arraySize) const {
  for (int i = 0; i < arraySize; ++i) {
    if (transitionArray[i].from == transition.from &&
        transitionArray[i].to == transition.to &&
        transitionArray[i].event_id == transition.event_id) {
      return true;
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////
/*
 * Set the finished handler.
 */

void SimpleFSM::setFinishedHandler(CallbackFunction f) {
  finished_cb = f;
}

/////////////////////////////////////////////////////////////////
/*
 * Get the time since the last transition.
 */

unsigned long SimpleFSM::lastTransitioned() const {
  return (last_transition == 0) ? 0 : (millis() - last_transition);
}

/////////////////////////////////////////////////////////////////
/*
* Check if the FSM is finished.
*/

bool SimpleFSM::isFinished() const {
  return is_finished;
}

/////////////////////////////////////////////////////////////////
/*
* Run the FSM.
* interval: The interval in milliseconds.
* tick_cb: A callback function that is called on every tick.
*/

void SimpleFSM::run(int interval /* = 1000 */, CallbackFunction tick_cb /* = NULL */) {
  unsigned long now = millis();
  // is the machine set up?
  if (!is_initialized) _initFSM();
  // are we ok?
  if (current_state == NULL) return;
  // is it time?
  if (!_isTimeForRun(now, interval)) return;
  // are we done yet?
  if (is_finished) return;
  // save the time
  last_run = now;
  // go through the timed events
  _handleTimedEvents(now);
  // trigger the on_state event
  if (current_state->on_state != NULL) current_state->on_state();
  // trigger the regular tick event
  if (tick_cb != NULL) tick_cb();
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::_isTimeForRun(unsigned long now, int interval) {
  return now >= last_run + interval;
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::_handleTimedEvents(unsigned long now) {
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
}

/////////////////////////////////////////////////////////////////
/*
* Initialize the FSM.
*/

bool SimpleFSM::_initFSM() {
  if (is_initialized) return false;
  is_initialized = true;
  if (inital_state == NULL) return false;
  return _changeToState(inital_state, millis());
}

/////////////////////////////////////////////////////////////////
/*
 * Change to a new state.
 */

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
/*
 * Get the DOT definition of the FSM.
 */

String SimpleFSM::getDotDefinition() {
  return "digraph G {\n" + _dot_header() + dot_definition + _dot_active_node() + _dot_inital_state() + "}\n";
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
