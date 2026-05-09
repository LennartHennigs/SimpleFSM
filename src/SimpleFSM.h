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

// Error codes for better error handling
enum class FSMError {
  OK,
  OUT_OF_MEMORY,
  INVALID_PARAMETER,
  ARRAY_TOO_LARGE
};

/////////////////////////////////////////////////////////////////

// forward declaration
class FSMTestHelper;

/////////////////////////////////////////////////////////////////


class SimpleFSM {
  public:
    SimpleFSM();
    SimpleFSM(State* initial_state);
    ~SimpleFSM();

    FSMError add(Transition t[], int size);
    FSMError add(TimedTransition t[], int size);
    FSMError add(State* states[], int size);

    // Global transition helpers - transitions that work from any state
    FSMError addGlobalTransition(State* to, int event_id);
    FSMError addGlobalTransition(State* to, int event_id, CallbackFunction callback);
    FSMError addGlobalTimedTransition(State* to, unsigned long interval);
    FSMError addGlobalTimedTransition(State* to, unsigned long interval, CallbackFunction callback);

    void setInitialState(State* state);
    void setFinishedHandler(CallbackFunction f);
    void setTransitionHandler(CallbackFunction f);

    bool trigger(int event_id);
    void run(int interval = DEFAULT_RUN_INTERVAL_MS, CallbackFunction tick_cb = NULL);
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

    // Error handling methods
    FSMError getLastError() const;
    bool hasError() const;
    const char* getErrorString(FSMError error) const;

 protected:
    // Safety limits to prevent excessive memory allocation
    static constexpr int MAX_TRANSITIONS = 100;
    static constexpr int MAX_TIMED_TRANSITIONS = 50;
    static constexpr int MAX_STATES = 50;
    
    // Default timing constants
    static constexpr int DEFAULT_RUN_INTERVAL_MS = 1000;
    
    // DOT graph formatting constants
    static constexpr const char* DOT_NODE_WIDTH = "1.5";
    static constexpr const char* DOT_PAD_VALUE = "0.5";
    
    // Reset/initialization values
    static constexpr unsigned long TIMESTAMP_RESET_VALUE = 0;
    static constexpr int INITIAL_ID_VALUE = 0;
    static constexpr int ARRAY_INCREMENT = 1;
    bool isSetupOK() const;
    int num_timed = INITIAL_ID_VALUE;
    int num_standard = INITIAL_ID_VALUE;
    int num_states = INITIAL_ID_VALUE;
    Transition* transitions = NULL;
    TimedTransition* timed = NULL;
    State** states = NULL;

    bool is_initialized = false;
    bool is_finished = false;
    unsigned long last_run = TIMESTAMP_RESET_VALUE;
    unsigned long last_transition = TIMESTAMP_RESET_VALUE;
    AbstractTransition* last_transition_ptr = NULL;

    // Error tracking
    FSMError last_error = FSMError::OK;

    State* initial_state = NULL;
    State* current_state = NULL;
    State* prev_state = NULL;
    CallbackFunction on_transition_cb = NULL;
    CallbackFunction finished_cb = NULL;

    String dot_definition = "";

    friend class FSMTestHelper;

    FSMError addUniqueState(State* state);

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
    String getDOTInitialState();
    String getDOTActiveNode();


};

/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////
