/////////////////////////////////////////////////////////////////
#include "SimpleFSM.h"

#include "State.h"
#include "Transitions.h"
/////////////////////////////////////////////////////////////////

SimpleFSM::SimpleFSM() {
}

/////////////////////////////////////////////////////////////////

SimpleFSM::SimpleFSM(State* initial_state) {
  setInitialState(initial_state);
}

/////////////////////////////////////////////////////////////////

FSMError SimpleFSM::getLastError() const {
  return last_error;
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::hasError() const {
  return last_error != FSMError::OK;
}

/////////////////////////////////////////////////////////////////

const char* SimpleFSM::getErrorString(FSMError error) const {
  switch (error) {
    case FSMError::OK: return "No error";
    case FSMError::OUT_OF_MEMORY: return "Out of memory";
    case FSMError::INVALID_PARAMETER: return "Invalid parameter";
    case FSMError::ARRAY_TOO_LARGE: return "Array size exceeds limits";
    default: return "Unknown error";
  }
}

/////////////////////////////////////////////////////////////////

SimpleFSM::~SimpleFSM() {
  if (states) {
    delete[] states;
    states = NULL;
  }
  if (transitions) {
    delete[] transitions;
    transitions = NULL;
  }
  if (timed) {
    delete[] timed;
    timed = NULL;
  }
}

/////////////////////////////////////////////////////////////////

int SimpleFSM::getTransitionCount() const {
  return num_standard;
}

/////////////////////////////////////////////////////////////////

int SimpleFSM::getTimedTransitionCount() const {
  return num_timed;
}

/////////////////////////////////////////////////////////////////

int SimpleFSM::getStateCount() const {
  return num_states;
}

/////////////////////////////////////////////////////////////////

State* SimpleFSM::getStateByName(String name) {
  for (int i = 0; i < num_states; ++i) {
    if (states[i]->getName() == name) {
      return states[i];
    } 
  }
  return NULL;
}

/////////////////////////////////////////////////////////////////

FSMError SimpleFSM::add(State* newStates[], int size) {
  // Validate input parameters
  if (newStates == NULL || size <= INITIAL_ID_VALUE) {
    last_error = FSMError::INVALID_PARAMETER;
    return last_error;
  }
  
  // Check bounds before processing
  if (num_states + size > MAX_STATES) {
    last_error = FSMError::ARRAY_TOO_LARGE;
    return last_error;
  }
  
  for (int i = 0; i < size; ++i) {
    FSMError result = addUniqueState(newStates[i]);
    if (result != FSMError::OK) {
      return result;
    }
  }
  
  last_error = FSMError::OK;
  return last_error;
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::reset() {
  is_initialized = false;
  is_finished = false;
  last_run = TIMESTAMP_RESET_VALUE;
  last_transition = TIMESTAMP_RESET_VALUE;
  last_error = FSMError::OK;  // Reset error state
  current_state = NULL;
  prev_state = NULL;

  for (int i = 0; i < num_timed; i++) {
    timed[i].reset();
  }
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::setInitialState(State* state) {
  initial_state = state;
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::trigger(int event_id) {
  if (!is_initialized) initFSM();
  // Find the transition with the current state and given event
  for (int i = 0; i < num_standard; i++) {
    // A NULL 'from' is only a genuine global transition when no source name
    // was declared. A non-empty fromStateName with a NULL pointer means the
    // name failed to resolve; such a transition must stay inert, not global.
    bool isGlobal = (transitions[i].from == NULL && transitions[i].fromStateName == "");
    bool matchesState = (transitions[i].from != NULL && transitions[i].from == current_state);
    if ((isGlobal || matchesState) &&
        transitions[i].event_id == event_id) {
      return transitionTo(&(transitions[i]));
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
  return current_state != NULL ? current_state : initial_state;
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::isInState(State* t) const {
  return t == getState();
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::setTransitionHandler(CallbackFunction f) {
  on_transition_cb = f;
}

/////////////////////////////////////////////////////////////////

// Change: states is now an array of State* (pointers), not State objects
// Add a state to the global states array if not already present
FSMError SimpleFSM::addUniqueState(State* state) {
  if (state == NULL) {
    // NULL state is valid for global transitions - just skip adding it
    return FSMError::OK;
  }
  
  // Check if already present
  for (int i = 0; i < num_states; i++) {
    if (states[i] == state) {
      return FSMError::OK; // Already present, not an error
    }
  }
  
  // Check bounds
  if (num_states >= MAX_STATES) {
    last_error = FSMError::ARRAY_TOO_LARGE;
    return last_error;
  }
  
  // Expand the states array
  State** temp = new State*[num_states + ARRAY_INCREMENT];
  if (temp == NULL) {
    last_error = FSMError::OUT_OF_MEMORY;
    return last_error;
  }
  
  if (states != NULL) {
    memcpy((void*)temp, states, num_states * sizeof(State*));
    delete[] states;
  }
  temp[num_states] = state;
  states = temp;
  num_states++;
  
  last_error = FSMError::OK;
  return last_error;
}

FSMError SimpleFSM::add(Transition newTransitions[], int size) {
  // Validate input parameters
  if (newTransitions == NULL || size <= INITIAL_ID_VALUE) {
    last_error = FSMError::INVALID_PARAMETER;
    return last_error;
  }
  
  // Check bounds - prevent excessive memory allocation
  if (num_standard + size > MAX_TRANSITIONS) {
    last_error = FSMError::ARRAY_TOO_LARGE;
    return last_error;
  }
  
  // Count the number of unique transitions
  int uniqueCount = 0;
  for (int i = 0; i < size; ++i) {
    if (!isDuplicate(newTransitions[i], transitions, num_standard) && 
        !isDuplicate(newTransitions[i], newTransitions, i)) {
      uniqueCount++;
    }
    // Add unique states globally - check for errors
    FSMError stateError = addUniqueState(newTransitions[i].from);
    if (stateError != FSMError::OK) {
      return stateError;
    }
    stateError = addUniqueState(newTransitions[i].to);
    if (stateError != FSMError::OK) {
      return stateError;
    }
  }
  
  // Final bounds check with actual unique count
  if (num_standard + uniqueCount > MAX_TRANSITIONS) {
    last_error = FSMError::ARRAY_TOO_LARGE;
    return last_error;
  }
  
  // Allocate or expand storage for transitions with exact size
  Transition* temp = new Transition[num_standard + uniqueCount];
  if (temp == NULL) {
    last_error = FSMError::OUT_OF_MEMORY;
    return last_error;
  }
  
  if (transitions != NULL) {
    // Use proper copy construction instead of memcpy to avoid vtable corruption
    for (int i = 0; i < num_standard; i++) {
      temp[i] = transitions[i];
    }
    delete[] transitions;
  }
  transitions = temp;
  
  // Add new transitions, avoiding duplicates
  for (int i = 0; i < size; ++i) {
    if (!isDuplicate(newTransitions[i], transitions, num_standard) && 
        !isDuplicate(newTransitions[i], newTransitions, i)) {
      transitions[num_standard] = newTransitions[i];
      addDOTTransition(transitions[num_standard]);
      num_standard++;
    }
  }
  
  last_error = FSMError::OK;
  return last_error;
}

FSMError SimpleFSM::add(TimedTransition newTransitions[], int size) {
  // Validate input parameters
  if (newTransitions == NULL || size <= INITIAL_ID_VALUE) {
    last_error = FSMError::INVALID_PARAMETER;
    return last_error;
  }
  
  // Check bounds - prevent excessive memory allocation
  if (num_timed + size > MAX_TIMED_TRANSITIONS) {
    last_error = FSMError::ARRAY_TOO_LARGE;
    return last_error;
  }
  
  // Count the number of unique transitions
  int uniqueCount = 0;
  for (int i = 0; i < size; ++i) {
    if (!isDuplicate(newTransitions[i], timed, num_timed) && 
        !isDuplicate(newTransitions[i], newTransitions, i)) {
      uniqueCount++;
    }
    // Add unique states globally - check for errors
    FSMError stateError = addUniqueState(newTransitions[i].from);
    if (stateError != FSMError::OK) {
      return stateError;
    }
    stateError = addUniqueState(newTransitions[i].to);
    if (stateError != FSMError::OK) {
      return stateError;
    }
  }
  
  // Final bounds check with actual unique count
  if (num_timed + uniqueCount > MAX_TIMED_TRANSITIONS) {
    last_error = FSMError::ARRAY_TOO_LARGE;
    return last_error;
  }
  
  // Allocate memory or expand existing storage with exact size
  TimedTransition* temp = new TimedTransition[num_timed + uniqueCount];
  if (temp == NULL) {
    last_error = FSMError::OUT_OF_MEMORY;
    return last_error;
  }
  
  if (timed != NULL) {
    // Use proper copy construction instead of memcpy to avoid vtable corruption
    for (int i = 0; i < num_timed; i++) {
      temp[i] = timed[i];
    }
    delete[] timed;
  }
  timed = temp;
  
  // Add new transitions while avoiding duplicates
  for (int i = 0; i < size; ++i) {
    if (!isDuplicate(newTransitions[i], timed, num_timed) && 
        !isDuplicate(newTransitions[i], newTransitions, i)) {
      timed[num_timed] = newTransitions[i];
      addDOTTransition(timed[num_timed]);
      num_timed++;
    }
  }
  
  last_error = FSMError::OK;
  return last_error;
}

/////////////////////////////////////////////////////////////////

FSMError SimpleFSM::addGlobalTransition(State* to, int event_id) {
  return addGlobalTransition(to, event_id, NULL);
}

/////////////////////////////////////////////////////////////////

FSMError SimpleFSM::addGlobalTransition(State* to, int event_id, CallbackFunction callback) {
  // Validate input parameters
  if (to == NULL) {
    last_error = FSMError::INVALID_PARAMETER;
    return last_error;
  }
  
  // Check bounds
  if (num_standard >= MAX_TRANSITIONS) {
    last_error = FSMError::ARRAY_TOO_LARGE;
    return last_error;
  }
  
  // Create a global transition using NULL as source state
  Transition globalTransition(NULL, to, event_id, callback);
  
  // Reject any existing global transition with the same event ID, regardless of target.
  // Two globals sharing an event ID would be ambiguous — only the first would ever fire.
  // isDuplicate() is not used here because it also matches on 'to', which we intentionally ignore.
  if (transitions != NULL) {
    for (int i = 0; i < num_standard; i++) {
      if (transitions[i].from == NULL && transitions[i].event_id == event_id) {
        last_error = FSMError::INVALID_PARAMETER;
        return last_error;
      }
    }
  }

  // Add the destination state if not already present
  FSMError stateError = addUniqueState(to);
  if (stateError != FSMError::OK) {
    return stateError;
  }
  
  // Allocate memory for the new transition
  Transition* temp = new Transition[num_standard + ARRAY_INCREMENT];
  if (temp == NULL) {
    last_error = FSMError::OUT_OF_MEMORY;
    return last_error;
  }
  
  // Copy existing transitions if any
  if (transitions != NULL) {
    for (int i = 0; i < num_standard; i++) {
      temp[i] = transitions[i];
    }
    delete[] transitions;
  }
  
  // Add the new global transition
  temp[num_standard] = globalTransition;
  transitions = temp;
  addDOTTransition(transitions[num_standard]);
  num_standard++;
  
  last_error = FSMError::OK;
  return last_error;
}

/////////////////////////////////////////////////////////////////

FSMError SimpleFSM::addGlobalTimedTransition(State* to, unsigned long interval) {
  return addGlobalTimedTransition(to, interval, NULL);
}

/////////////////////////////////////////////////////////////////

FSMError SimpleFSM::addGlobalTimedTransition(State* to, unsigned long interval, CallbackFunction callback) {
  // Validate input parameters
  if (to == NULL || interval == 0) {
    last_error = FSMError::INVALID_PARAMETER;
    return last_error;
  }
  
  // Check bounds
  if (num_timed >= MAX_TIMED_TRANSITIONS) {
    last_error = FSMError::ARRAY_TOO_LARGE;
    return last_error;
  }
  
  // Create a global timed transition using NULL as source state
  TimedTransition globalTimedTransition(NULL, to, interval, callback);
  
  if (isDuplicate(globalTimedTransition, timed, num_timed)) {
    last_error = FSMError::INVALID_PARAMETER;
    return last_error;
  }

  // Add the destination state if not already present
  FSMError stateError = addUniqueState(to);
  if (stateError != FSMError::OK) {
    return stateError;
  }
  
  // Allocate memory for the new timed transition
  TimedTransition* temp = new TimedTransition[num_timed + ARRAY_INCREMENT];
  if (temp == NULL) {
    last_error = FSMError::OUT_OF_MEMORY;
    return last_error;
  }
  
  // Copy existing timed transitions if any
  if (timed != NULL) {
    for (int i = 0; i < num_timed; i++) {
      temp[i] = timed[i];
    }
    delete[] timed;
  }
  
  // Add the new global timed transition
  temp[num_timed] = globalTimedTransition;
  timed = temp;
  addDOTTransition(timed[num_timed]);
  num_timed++;
  
  last_error = FSMError::OK;
  return last_error;
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::isDuplicate(const TimedTransition& transition, const TimedTransition* transitionArray, int arraySize) const {
  for (int i = 0; i < arraySize; ++i) {
    // Compare the declared state names too: string-based transitions share
    // NULL from/to pointers until they are resolved, so pointers alone would
    // wrongly collapse distinct named transitions into one.
    if (transitionArray[i].from == transition.from &&
        transitionArray[i].to == transition.to &&
        transitionArray[i].fromStateName == transition.fromStateName &&
        transitionArray[i].toStateName == transition.toStateName &&
        transitionArray[i].interval == transition.interval) {
      return true;
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::isDuplicate(const Transition& transition, const Transition* transitionArray, int arraySize) const {
  for (int i = 0; i < arraySize; ++i) {
    if (transitionArray[i].from == transition.from &&
        transitionArray[i].to == transition.to &&
        transitionArray[i].fromStateName == transition.fromStateName &&
        transitionArray[i].toStateName == transition.toStateName &&
        transitionArray[i].event_id == transition.event_id) {
      return true;
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::setFinishedHandler(CallbackFunction f) {
  finished_cb = f;
}

/////////////////////////////////////////////////////////////////

unsigned long SimpleFSM::lastTransitioned() const {
  return (last_transition == TIMESTAMP_RESET_VALUE) ? TIMESTAMP_RESET_VALUE : (millis() - last_transition);
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::isFinished() const {
  return is_finished;
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::isSetupOK() const {
  return ((current_state != NULL) && (num_states > 0));
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::run(int interval /* = DEFAULT_RUN_INTERVAL_MS */, CallbackFunction tick_cb /* = NULL */) {
  unsigned long now = millis();
  // is the machine set up?
  if (!is_initialized) initFSM();
  // are we ok?
  if (!isSetupOK()) return;
  // are we done yet?
  if (is_finished) return;
  // is it time?
  if (!isTimeForRun(now, interval)) return;
  // save the time
  last_run = now;
  // go through the timed events
  handleTimedEvents(now);
  // trigger the on_state event
  if (current_state->on_state != NULL) current_state->on_state();
  // trigger the regular tick event
  if (tick_cb != NULL) tick_cb();
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::isTimeForRun(unsigned long now, int interval) {
  // Wrap-safe: subtracting unsigned timestamps stays correct across the
  // ~49-day millis() rollover, whereas (last_run + interval) can overflow.
  return (now - last_run) >= (unsigned long)interval;
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::handleTimedEvents(unsigned long now) {
  for (int i = 0; i < num_timed; i++) {
    // Check for global timed transition (from == NULL) OR specific state transition
    if (timed[i].from != NULL && timed[i].from != current_state) continue;
    // A NULL 'from' with a declared source name is an unresolved name, not a
    // global transition — skip it so a typo does not fire from every state.
    if (timed[i].from == NULL && timed[i].fromStateName != "") continue;
    
    // For global timed transitions (from == NULL), they apply to all states
    // For specific transitions, they only apply when from == current_state
    
    // start the transition timer 
    if (timed[i].start == 0) {
      timed[i].start = now;
      continue;
    }
    // reached the interval?
    if (now - timed[i].start >= timed[i].interval) {
      if (transitionTo(&timed[i])) {
        timed[i].start = 0;
        return;
      }
    }
  }
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::checkAndInitializeTransitions() {
  for (int i=0; i < num_standard; i++) {
    if (transitions[i].fromStateName != "") transitions[i].from = getStateByName(transitions[i].fromStateName);
    if (transitions[i].toStateName != "") transitions[i].to = getStateByName(transitions[i].toStateName);
  }

  for (int i=0; i < num_timed; i++) {
    if (timed[i].fromStateName != "") timed[i].from = getStateByName(timed[i].fromStateName);
    if (timed[i].toStateName != "") timed[i].to = getStateByName(timed[i].toStateName);
  }
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::initFSM() {
  if (is_initialized) return false;
  checkAndInitializeTransitions();
  is_initialized = true;
  return changeToState(initial_state, millis());
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::changeToState(State* s, unsigned long now) {
  if (s == NULL) return false;
  // set the new state
  prev_state = current_state;
  current_state = s;
  if (s->on_enter != NULL) s->on_enter();
  // save the time
  last_run = now;
  last_transition = now;
  // Reset timers for timed transitions from the new state
  for (int i = 0; i < num_timed; i++) {
    // Reset timers for transitions FROM this state or global transitions (from == NULL)
    if (timed[i].from == current_state || timed[i].from == NULL) {
      timed[i].start = 0;
    }
  }
  // is this the end?
  if (s->is_final && finished_cb != NULL) finished_cb();
  if (s->is_final) is_finished = true;
  return true;
}

AbstractTransition* SimpleFSM::getLastTransition() const {
  return last_transition_ptr;  
}
/////////////////////////////////////////////////////////////////

String SimpleFSM::getDotDefinition(bool showActive /* = TRUE */ ) {
  return "digraph G {\n" + getDOTHeader() + dot_definition + (showActive ? getDOTActiveNode() : "") + getDOTInitialState() + "}\n";
}

/////////////////////////////////////////////////////////////////

bool SimpleFSM::transitionTo(AbstractTransition* transition) {
  // empty parameter?
  if (transition->to == NULL) return false;
  // can I pass the guard
  if (transition->guard_cb != NULL && !transition->guard_cb()) return false;
  // trigger events - handle global transitions (from == NULL)
  if (transition->from != NULL && transition->from->on_exit != NULL) {
    transition->from->on_exit();
  } else if (transition->from == NULL && current_state != NULL && current_state->on_exit != NULL) {
    // For global transitions, trigger current state's on_exit
    current_state->on_exit();
  }
  if (transition->on_run_cb != NULL) transition->on_run_cb();
  if (on_transition_cb != NULL) on_transition_cb();
  // store the transition
  last_transition_ptr = transition;
  return changeToState(transition->to, millis());
}

/////////////////////////////////////////////////////////////////

String SimpleFSM::getDOTTransition(String from, String to, String label, String param) {
  return "\t\"" + from + "\" -> \"" + to + "\"" + " [label=\"" + label + " (" + param + ")\"];\n";
}

/////////////////////////////////////////////////////////////////

String SimpleFSM::getDOTInitialState() {
  return initial_state ? "\t\"" + initial_state->getName() + "\" [style=filled fontcolor=white fillcolor=black];\n\n" : "";
}

/////////////////////////////////////////////////////////////////

String SimpleFSM::getDOTActiveNode() {
  return current_state ? "\t\"" + current_state->getName() + "\" [style=filled fontcolor=white];\n" : "";
}

/////////////////////////////////////////////////////////////////

String SimpleFSM::getDOTHeader() {
  return String("\trankdir=LR; pad=") + DOT_PAD_VALUE + 
         String("\n\tnode [shape=circle fixedsize=true width=") + DOT_NODE_WIDTH + String("];\n");
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::addDOTTransition(Transition& t) {
  // Names may not be resolved to pointers yet (string-based transitions),
  // so fall back to the declared state names before dereferencing.
  String fromName = (t.from != NULL) ? t.from->getName() : (t.fromStateName != "" ? t.fromStateName : "GLOBAL");
  String toName = (t.to != NULL) ? t.to->getName() : t.toStateName;
  dot_definition = dot_definition + getDOTTransition(fromName, toName, t.getName(), "ID=" + String(t.event_id));
}

/////////////////////////////////////////////////////////////////

void SimpleFSM::addDOTTransition(TimedTransition& t) {
  String fromName = (t.from != NULL) ? t.from->getName() : (t.fromStateName != "" ? t.fromStateName : "GLOBAL");
  String toName = (t.to != NULL) ? t.to->getName() : t.toStateName;
  dot_definition = dot_definition + getDOTTransition(fromName, toName, t.getName(), String(t.getInterval()) + "ms");
}

/////////////////////////////////////////////////////////////////
