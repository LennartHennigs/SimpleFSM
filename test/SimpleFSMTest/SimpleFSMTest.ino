// SimpleFSM Comprehensive Test Suite
// Complete test coverage for all FSM functionality

#include <AUnitVerbose.h>
#include <SimpleFSM.h>
#include <FSMUtils.h>
#include "FSMTestHelper.h"

using namespace aunit;

//////////////////////////////////////////////////////////////////
// Test Utility Functions and Global Variables
//////////////////////////////////////////////////////////////////

// Callback counters for testing
static int g_enter_count = 0;
static int g_state_count = 0; 
static int g_exit_count = 0;
static int g_transition_count = 0;
static int g_timer_count = 0;
static int g_finished_count = 0;
static int g_guard_count = 0;
static bool g_guard_result = true;

// Test callback functions
void onEnterCallback() { g_enter_count++; }
void onStateCallback() { g_state_count++; }
void onExitCallback() { g_exit_count++; }
void onTransitionCallback() { g_transition_count++; }
void onTimerCallback() { g_timer_count++; }
void onFinishedCallback() { g_finished_count++; }
bool guardCondition() { g_guard_count++; return g_guard_result; }

// Reset all counters before each test
void resetCounters() {
    g_enter_count = 0;
    g_state_count = 0;
    g_exit_count = 0;
    g_transition_count = 0;
    g_timer_count = 0;
    g_finished_count = 0;
    g_guard_count = 0;
    g_guard_result = true;
}

//////////////////////////////////////////////////////////////////
// Phase 1: State Management Tests
//////////////////////////////////////////////////////////////////

test(StateManagement, BasicStateCreation) {
    resetCounters();
    
    State state1("State1", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &state1 };
    FSMError result = fsm.add(states, 1);
    
    assertEqual((int)result, (int)FSMError::OK);
    assertEqual(fsm.getStateCount(), 1);
    assertEqual(state1.getName(), "State1");
    assertFalse(state1.isFinal());
}

test(StateManagement, StateCallbacks) {
    resetCounters();
    
    State state1("State1", onEnterCallback, onStateCallback, onExitCallback);
    State state2("State2", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    fsm.setInitialState(&state1);
    
    // Initialize FSM to trigger on_enter callback
    fsm.run(10); 
    assertEqual(g_enter_count, 1); // on_enter should be called during initialization
    
    // The first run() call sets last_run, so on_state won't be called until next run()
    // Wait and call run() again to trigger on_state callback
    delay(15); // Wait longer than the run interval (10ms)
    fsm.run(10);
    assertEqual(g_state_count, 1); // Should be called during the second run() call
    
    // Test transition with on_exit callback
    Transition trans(&state1, &state2, 1);
    fsm.add(&trans, 1);
    
    fsm.trigger(1);
    assertEqual(g_exit_count, 1); // state1 on_exit called
    assertEqual(g_enter_count, 2); // state2 on_enter called (now 2 total)
}

test(StateManagement, FinalState) {
    resetCounters();
    
    State running("Running", onEnterCallback);
    State finished("Finished", onEnterCallback, NULL, NULL, true);
    SimpleFSM fsm;
    
    State* states[] = { &running, &finished };
    fsm.add(states, 2);
    fsm.setInitialState(&running);
    fsm.setFinishedHandler(onFinishedCallback);
    
    assertTrue(finished.isFinal()); // Should be final from constructor
    
    // Transition to final state
    Transition trans(&running, &finished, 1);
    fsm.add(&trans, 1);
    
    assertFalse(fsm.isFinished());
    fsm.trigger(1);
    
    assertTrue(fsm.isFinished());
    assertEqual(g_finished_count, 1);
}

test(StateManagement, StateProperties) {
    resetCounters();
    
    State state1;
    state1.setup("TestState", onEnterCallback, onStateCallback, onExitCallback, false);
    
    assertEqual(state1.getName(), "TestState");
    assertFalse(state1.isFinal());
    assertTrue(state1.getID() > 0); // Should have positive ID
    
    // Test setters
    state1.setName("NewName");
    assertEqual(state1.getName(), "NewName");
    
    state1.setAsFinal(true);
    assertTrue(state1.isFinal());
}

test(StateManagement, MultipleStates) {
    resetCounters();
    
    const int STATE_COUNT = 10;
    State states[STATE_COUNT];
    State* state_ptrs[STATE_COUNT];
    
    // Create multiple states
    for (int i = 0; i < STATE_COUNT; i++) {
        states[i].setup("State" + String(i), onEnterCallback);
        state_ptrs[i] = &states[i];
    }
    
    SimpleFSM fsm;
    FSMError result = fsm.add(state_ptrs, STATE_COUNT);
    
    assertEqual((int)result, (int)FSMError::OK);
    assertEqual(fsm.getStateCount(), STATE_COUNT);
}

//////////////////////////////////////////////////////////////////
// Phase 2: Transition Tests  
//////////////////////////////////////////////////////////////////

test(TransitionTests, BasicTransition) {
    resetCounters();
    
    State idle("Idle", onEnterCallback);
    State running("Running", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &idle, &running };
    fsm.add(states, 2);
    fsm.setInitialState(&idle);
    
    Transition trans(&idle, &running, 1, onTransitionCallback);
    fsm.add(&trans, 1);
    
    assertEqual(fsm.getState(), &idle);
    assertTrue(fsm.trigger(1));
    assertEqual(fsm.getState(), &running);
    assertEqual(g_transition_count, 1);
}

test(TransitionTests, GuardConditions) {
    resetCounters();
    
    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback); 
    SimpleFSM fsm;
    
    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    fsm.setInitialState(&state1);
    
    Transition trans(&state1, &state2, 1, NULL, "", guardCondition);
    fsm.add(&trans, 1);
    
    // Test guard allows transition
    g_guard_result = true;
    assertTrue(fsm.trigger(1));
    assertEqual(fsm.getState(), &state2);
    assertEqual(g_guard_count, 1);
    
    // Reset and test guard blocks transition
    fsm.reset();
    fsm.setInitialState(&state1);
    resetCounters();
    g_guard_result = false; // Set AFTER resetCounters to avoid it being reset to true
    
    // Initialize FSM after reset
    fsm.run(10);
    assertEqual(fsm.getState(), &state1);
    
    assertFalse(fsm.trigger(1));
    assertEqual(fsm.getState(), &state1); // Should stay in state1
    assertEqual(g_guard_count, 1);
}

test(TransitionTests, MultipleTransitionsFromState) {
    resetCounters();
    
    State menu("Menu", onEnterCallback);
    State settings("Settings", onEnterCallback);
    State game("Game", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &menu, &settings, &game };
    fsm.add(states, 3);
    fsm.setInitialState(&menu);
    
    Transition transitions[] = {
        Transition(&menu, &settings, 1),
        Transition(&menu, &game, 2)
    };
    fsm.add(transitions, 2);
    
    // Test first transition
    assertTrue(fsm.trigger(1));
    assertEqual(fsm.getState(), &settings);
    
    // Reset and test second transition
    fsm.reset();
    fsm.setInitialState(&menu);
    assertTrue(fsm.trigger(2));
    assertEqual(fsm.getState(), &game);
}

test(TransitionTests, InvalidTransitions) {
    resetCounters();
    
    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    fsm.setInitialState(&state1);
    
    Transition trans(&state1, &state2, 1);
    fsm.add(&trans, 1);
    
    // Test valid transition
    assertTrue(fsm.trigger(1));
    
    // Test invalid event ID
    assertFalse(fsm.trigger(999));
    assertEqual(fsm.getState(), &state2); // Should stay in current state
}

//////////////////////////////////////////////////////////////////
// Phase 3: Timed Transition Tests
//////////////////////////////////////////////////////////////////

test(TimedTransitions, BasicTimedTransition) {
    resetCounters();
    
    State waiting("Waiting", onEnterCallback);
    State timeout("Timeout", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &waiting, &timeout };
    fsm.add(states, 2);
    fsm.setInitialState(&waiting);
    
    TimedTransition timed(&waiting, &timeout, 50, onTimerCallback);
    fsm.add(&timed, 1);
    
    fsm.run(10); // Initialize with small interval
    assertEqual(fsm.getState(), &waiting);
    assertEqual(fsm.getTimedTransitionCount(), 1);
    
    // Wait for timeout
    bool transitioned = false;
    for (int i = 0; i < 20 && !transitioned; i++) {
        fsm.run(10);
        if (fsm.getState() == &timeout) {
            transitioned = true;
        }
        delay(10);
    }
    
    assertTrue(transitioned);
    assertEqual(g_timer_count, 1);
}

test(TimedTransitions, MultipleTimedTransitions) {
    resetCounters();
    
    State idle("Idle", onEnterCallback);
    State sleep("Sleep", onEnterCallback);
    State deep_sleep("DeepSleep", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &idle, &sleep, &deep_sleep };
    fsm.add(states, 3);
    fsm.setInitialState(&idle);
    
    TimedTransition transitions[] = {
        TimedTransition(&idle, &sleep, 30, onTimerCallback),
        TimedTransition(&sleep, &deep_sleep, 50, onTimerCallback)
    };
    fsm.add(transitions, 2);
    
    // First transition
    bool first_transition = false;
    for (int i = 0; i < 20 && !first_transition; i++) {
        fsm.run(10);
        if (fsm.getState() == &sleep) {
            first_transition = true;
        }
        delay(5);
    }
    assertTrue(first_transition);
    
    // Second transition  
    bool second_transition = false;
    for (int i = 0; i < 30 && !second_transition; i++) {
        fsm.run(10);
        if (fsm.getState() == &deep_sleep) {
            second_transition = true;
        }
        delay(5);
    }
    assertTrue(second_transition);
    assertEqual(g_timer_count, 2);
}

test(TimedTransitions, TimedTransitionWithGuard) {
    resetCounters();
    
    State active("Active", onEnterCallback);
    State inactive("Inactive", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &active, &inactive };
    fsm.add(states, 2);
    fsm.setInitialState(&active);
    
    TimedTransition timed(&active, &inactive, 30, onTimerCallback, "", guardCondition);
    fsm.add(&timed, 1);
    
    // Test guard blocks timed transition
    g_guard_result = false;
    
    for (int i = 0; i < 15; i++) {
        fsm.run(10);
        delay(5);
    }
    
    assertEqual(fsm.getState(), &active); // Should stay in active
    assertEqual(g_timer_count, 0); // Timer callback shouldn't be called
    assertTrue(g_guard_count > 0); // Guard should be checked
}

//////////////////////////////////////////////////////////////////
// Phase 4: Global Transition Tests
//////////////////////////////////////////////////////////////////

test(GlobalTransitions, GlobalTransitionHelper) {
    Serial.println("DEBUG: Starting GlobalTransitions_GlobalTransitionHelper");
    resetCounters();
    
    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    State emergency("Emergency", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &state1, &state2, &emergency };
    fsm.add(states, 3);
    fsm.setInitialState(&state1);
    
    // Use fixed helper method
    FSMError result = fsm.addGlobalTransition(&emergency, 999, onTransitionCallback);
    assertEqual((int)result, (int)FSMError::OK);
    
    // Test from state1
    assertTrue(fsm.trigger(999));
    assertEqual(fsm.getState(), &emergency);
    assertEqual(g_transition_count, 1);
    
    // Reset and test from state2
    fsm.reset();
    fsm.setInitialState(&state2);
    resetCounters();
    
    assertTrue(fsm.trigger(999));
    assertEqual(fsm.getState(), &emergency);
    assertEqual(g_transition_count, 1);
}

test(GlobalTransitions, GlobalTimedTransition) {
    Serial.println("DEBUG: Starting GlobalTransitions_GlobalTimedTransition");
    resetCounters();
    
    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    State timeout_state("Timeout", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &state1, &state2, &timeout_state };
    fsm.add(states, 3);
    fsm.setInitialState(&state1);
    
    // Use fixed helper method
    FSMError result = fsm.addGlobalTimedTransition(&timeout_state, 40, onTimerCallback);
    assertEqual((int)result, (int)FSMError::OK);
    
    // Initialize FSM first
    fsm.run(10);
    assertEqual(fsm.getState(), &state1);
    assertEqual(fsm.getTimedTransitionCount(), 1);
    
    // Should timeout from any state
    bool transitioned = false;
    for (int i = 0; i < 20 && !transitioned; i++) {
        fsm.run(10);
        if (fsm.getState() == &timeout_state) {
            transitioned = true;
        }
        delay(5);
    }
    
    assertTrue(transitioned);
    assertEqual(g_timer_count, 1);
}

test(GlobalTransitions, TraditionalGlobalTransition) {
    Serial.println("DEBUG: Starting GlobalTransitions_TraditionalGlobalTransition");
    resetCounters();
    
    State normal("Normal", onEnterCallback);
    State error("Error", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &normal, &error };
    fsm.add(states, 2);
    fsm.setInitialState(&normal);
    
    // Traditional global transition with NULL source
    Transition global(NULL, &error, 999, onTransitionCallback);
    fsm.add(&global, 1);
    
    // Initialize FSM
    fsm.run(10);
    assertEqual(fsm.getState(), &normal);
    assertEqual(fsm.getTransitionCount(), 1); // Should have 1 transition
    
    assertTrue(fsm.trigger(999));
    assertEqual(fsm.getState(), &error);
    assertEqual(g_transition_count, 1);
}

//////////////////////////////////////////////////////////////////
// Phase 5: Error Handling & Edge Cases
//////////////////////////////////////////////////////////////////

test(ErrorHandling, InvalidParameters) {
    resetCounters();
    
    SimpleFSM fsm;
    
    // Test null pointer arrays
    FSMError result = fsm.add((State**)NULL, 1);
    assertEqual((int)result, (int)FSMError::INVALID_PARAMETER);
    assertTrue(fsm.hasError());
    assertEqual((int)fsm.getLastError(), (int)FSMError::INVALID_PARAMETER);
    
    // Test zero size
    State state1("State1", onEnterCallback);
    State* states[] = { &state1 };
    result = fsm.add(states, 0);
    assertEqual((int)result, (int)FSMError::INVALID_PARAMETER);
}

test(ErrorHandling, MemoryLimits) {
    resetCounters();
    
    SimpleFSM fsm;
    
    // Test state limit (MAX_STATES = 50)
    const int OVER_LIMIT = 55;
    State states[OVER_LIMIT];
    State* state_ptrs[OVER_LIMIT];
    
    for (int i = 0; i < OVER_LIMIT; i++) {
        states[i].setup("State" + String(i), onEnterCallback);
        state_ptrs[i] = &states[i];
    }
    
    FSMError result = fsm.add(state_ptrs, OVER_LIMIT);
    assertEqual((int)result, (int)FSMError::ARRAY_TOO_LARGE);
}

test(ErrorHandling, ErrorStringRetrieval) {
    resetCounters();
    
    SimpleFSM fsm;
    
    // Test error string retrieval
    const char* ok_str = fsm.getErrorString(FSMError::OK);
    assertNotEqual(ok_str, (const char*)NULL);
    
    const char* invalid_str = fsm.getErrorString(FSMError::INVALID_PARAMETER);
    assertNotEqual(invalid_str, (const char*)NULL);
    
    const char* memory_str = fsm.getErrorString(FSMError::OUT_OF_MEMORY);
    assertNotEqual(memory_str, (const char*)NULL);
}

test(ErrorHandling, DuplicateTransitions) {
    resetCounters();
    
    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    
    // Add same transition twice
    Transition trans1(&state1, &state2, 1);
    Transition trans2(&state1, &state2, 1); // Duplicate
    
    fsm.add(&trans1, 1);
    FSMError result = fsm.add(&trans2, 1);
    
    assertEqual((int)result, (int)FSMError::OK); // Duplicates are silently ignored
    assertEqual(fsm.getTransitionCount(), 1); // Should still be 1
}

//////////////////////////////////////////////////////////////////
// Phase 6: Advanced Features
//////////////////////////////////////////////////////////////////

test(AdvancedFeatures, FSMReset) {
    resetCounters();
    
    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    fsm.setInitialState(&state1);
    
    Transition trans(&state1, &state2, 1);
    fsm.add(&trans, 1);
    
    // Transition to state2
    fsm.trigger(1);
    assertEqual(fsm.getState(), &state2);
    
    // Reset FSM
    fsm.reset();
    assertEqual(fsm.getState(), &state1); // Should return to initial state
}

test(AdvancedFeatures, TransitionHandler) {
    resetCounters();
    
    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    fsm.setInitialState(&state1);
    fsm.setTransitionHandler(onTransitionCallback);
    
    Transition trans(&state1, &state2, 1);
    fsm.add(&trans, 1);
    
    fsm.trigger(1);
    assertEqual(g_transition_count, 1); // Global transition handler called
}

test(AdvancedFeatures, StateTracking) {
    resetCounters();
    
    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    fsm.setInitialState(&state1);
    
    Transition trans(&state1, &state2, 1);
    fsm.add(&trans, 1);
    
    // Test initial state tracking
    assertEqual(fsm.getState(), &state1);
    assertTrue(fsm.isInState(&state1));
    assertFalse(fsm.isInState(&state2));
    
    // Test transition and state tracking  
    fsm.trigger(1);
    
    assertEqual(fsm.getState(), &state2);
    assertTrue(fsm.isInState(&state2));
    assertFalse(fsm.isInState(&state1));
    assertEqual(fsm.getPreviousState(), &state1);
    // Skip timing test - it's not reliable in test environment
}

test(AdvancedFeatures, DOTGeneration) {
    resetCounters();
    
    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    fsm.setInitialState(&state1);
    
    Transition trans(&state1, &state2, 1);
    fsm.add(&trans, 1);
    
    String dot = fsm.getDotDefinition();
    assertTrue(dot.length() > 0);
    assertTrue(dot.indexOf("State1") >= 0);
    assertTrue(dot.indexOf("State2") >= 0);
}

//////////////////////////////////////////////////////////////////
// Phase 7: Integration & Real-world Scenarios
//////////////////////////////////////////////////////////////////

test(Integration, TrafficLightFSM) {
    Serial.println("DEBUG: Starting Integration_TrafficLightFSM");
    resetCounters();
    
    // Create traffic light states
    State red("Red", onEnterCallback);
    State yellow("Yellow", onEnterCallback);  
    State green("Green", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &red, &yellow, &green };
    fsm.add(states, 3);
    fsm.setInitialState(&red);
    
    // Create timed transitions
    TimedTransition transitions[] = {
        TimedTransition(&red, &green, 30),
        TimedTransition(&green, &yellow, 30),
        TimedTransition(&yellow, &red, 30)
    };
    fsm.add(transitions, 3);
    
    // Simple test - just verify setup
    fsm.run(10);
    assertEqual(fsm.getState(), &red);
    assertEqual(fsm.getTimedTransitionCount(), 3);
    
    // Test one transition
    bool reached_green = false;
    for (int i = 0; i < 10 && !reached_green; i++) {
        fsm.run(10);
        if (fsm.getState() == &green) reached_green = true;
        delay(10);
    }
    assertTrue(reached_green);
}

test(Integration, ComplexStateMachine) {
    Serial.println("DEBUG: Starting Integration_ComplexStateMachine");
    resetCounters();
    
    // Create a complex FSM with multiple transition types
    State boot("Boot", onEnterCallback);
    State idle("Idle", onEnterCallback);
    State error("Error", onEnterCallback);
    State shutdown("Shutdown", onEnterCallback, NULL, NULL, true);
    
    SimpleFSM fsm;
    State* states[] = { &boot, &idle, &error, &shutdown };
    fsm.add(states, 4);
    fsm.setInitialState(&boot);
    fsm.setFinishedHandler(onFinishedCallback);
    shutdown.setAsFinal(true);
    
    // Regular transitions
    Transition regular_trans[] = {
        Transition(&boot, &idle, 1)
    };
    fsm.add(regular_trans, 1);
    
    // Global transitions using fixed helper methods
    fsm.addGlobalTransition(&error, 999); // Emergency stop
    fsm.addGlobalTransition(&shutdown, 0); // Shutdown command
    
    // Test basic functionality
    fsm.run(10);
    assertEqual(fsm.getState(), &boot);
    
    // Test manual transition
    assertTrue(fsm.trigger(1)); // boot -> idle
    assertEqual(fsm.getState(), &idle);
    
    // Test global emergency
    assertTrue(fsm.trigger(999)); // idle -> error
    assertEqual(fsm.getState(), &error);
    
    // Test global shutdown
    assertTrue(fsm.trigger(0)); // error -> shutdown
    assertEqual(fsm.getState(), &shutdown);
    assertTrue(fsm.isFinished());
    assertEqual(g_finished_count, 1);
}

//////////////////////////////////////////////////////////////////
// Phase 8: GitHub Issues Tests
//////////////////////////////////////////////////////////////////

// Issue #26: FSMUtils Many-to-One Transitions
test(GitHubIssues, FSMUtilsManyToOneTransitions) {
    resetCounters();
    
    // Create multiple source states and one target
    State screen1("Screen1", onEnterCallback);
    State screen2("Screen2", onEnterCallback);  
    State screen3("Screen3", onEnterCallback);
    State home("Home", onEnterCallback);
    SimpleFSM fsm;
    
    // Add states to FSM
    State* states[] = { &screen1, &screen2, &screen3, &home };
    assertEqual((int)fsm.add(states, 4), (int)FSMError::OK);
    
    // Create many-to-one transitions using FSMUtils
    State* sources[] = { &screen1, &screen2, &screen3 };
    Transition homeTransitions[3];
    FSMUtils::createManyToOneTransitions(sources, 3, &home, 1, homeTransitions);
    
    // Add the generated transitions
    assertEqual((int)fsm.add(homeTransitions, 3), (int)FSMError::OK);
    
    // Test from screen1 -> home
    fsm.setInitialState(&screen1);
    fsm.run(10);
    assertEqual(fsm.getState(), &screen1);
    assertTrue(fsm.trigger(1));
    assertEqual(fsm.getState(), &home);
    
    // Test from screen2 -> home
    FSMTestHelper::changeToState(fsm, &screen2);
    assertEqual(fsm.getState(), &screen2);
    assertTrue(fsm.trigger(1));
    assertEqual(fsm.getState(), &home);
    
    // Test from screen3 -> home  
    FSMTestHelper::changeToState(fsm, &screen3);
    assertEqual(fsm.getState(), &screen3);
    assertTrue(fsm.trigger(1));
    assertEqual(fsm.getState(), &home);
    
    // Verify all states were entered (3 screens + 3 home entries)
    assertEqual(g_enter_count, 6);
}

// Issue #26: FSMUtils Many-to-One Timed Transitions
test(GitHubIssues, FSMUtilsManyToOneTimedTransitions) {
    resetCounters();
    
    State screen1("Screen1", onEnterCallback);
    State screen2("Screen2", onEnterCallback);
    State screen3("Screen3", onEnterCallback);
    State home("Home", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &screen1, &screen2, &screen3, &home };
    assertEqual((int)fsm.add(states, 4), (int)FSMError::OK);
    
    // Create many-to-one timed transitions
    State* sources[] = { &screen1, &screen2, &screen3 };
    TimedTransition timedHomeTransitions[3];
    FSMUtils::createManyToOneTimedTransitions(sources, 3, &home, 50, timedHomeTransitions);
    
    assertEqual((int)fsm.add(timedHomeTransitions, 3), (int)FSMError::OK);
    
    // Test timed transition from screen1
    fsm.setInitialState(&screen1);
    fsm.run(10);
    assertEqual(fsm.getState(), &screen1);
    
    // Wait for timeout
    bool transitioned = false;
    for (int i = 0; i < 20 && !transitioned; i++) {
        fsm.run(10);
        if (fsm.getState() == &home) {
            transitioned = true;
        }
        delay(5);
    }
    assertTrue(transitioned);
    assertEqual(fsm.getState(), &home);
}

// Issue #25: TimedTransition Timer Reset Bug
test(GitHubIssues, TimedTransitionTimerResetBug) {
    resetCounters();
    
    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    State state4("State4", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &state1, &state2, &state4 };
    assertEqual((int)fsm.add(states, 3), (int)FSMError::OK);
    
    // Create transitions that mimic the bug scenario
    Transition manual_trans[] = {
        Transition(&state1, &state2, 1), // Manual button trigger
        Transition(&state2, &state1, 2)  // Return transition
    };
    assertEqual((int)fsm.add(manual_trans, 2), (int)FSMError::OK);
    
    // Timed transition that should reset when state changes
    TimedTransition timed_trans[] = {
        TimedTransition(&state1, &state4, 100) // Should be 100ms from state1 entry
    };
    assertEqual((int)fsm.add(timed_trans, 1), (int)FSMError::OK);
    
    fsm.setInitialState(&state1);
    fsm.run(10);
    assertEqual(fsm.getState(), &state1);
    
    // Wait 30ms, then manually transition to state2
    delay(30);
    assertTrue(fsm.trigger(1)); // state1 -> state2
    assertEqual(fsm.getState(), &state2);
    
    // Immediately return to state1 (this should reset the timer)
    assertTrue(fsm.trigger(2)); // state2 -> state1  
    assertEqual(fsm.getState(), &state1);
    
    // Record timer reset point
    unsigned long reset_time = millis();
    
    // Use loop-based waiting with tolerance instead of fixed delays
    // Wait approximately 80ms from reset point
    unsigned long target_time = reset_time + 80;
    while (millis() < target_time) {
        fsm.run(10);
        delay(5);
    }
    
    // Should still be in state1 (timer should have been reset)
    assertEqual(fsm.getState(), &state1);
    
    // Wait for the reset timer to expire (additional 30ms + tolerance)
    target_time = reset_time + 110; // Total 110ms from reset point
    while (millis() < target_time) {
        fsm.run(10);
        delay(5);
    }
    
    // Give a bit more time to ensure transition completes
    fsm.run(10);
    delay(10);
    fsm.run(10);
    
    // Now should transition to state4
    assertEqual(fsm.getState(), &state4);
}

// Issue #27: getName() String Corruption Test
test(GitHubIssues, GetNameStringCorruption) {
    resetCounters();
    
    // Create states with various name lengths and characters to test string corruption
    State states[6] = {
        State("State0", onEnterCallback),
        State("State1", onEnterCallback), 
        State("State2", onEnterCallback),
        State("State3", onEnterCallback),
        State("LongStateName4WithSpecialChars!@#", onEnterCallback),
        State("EvenLongerStateName5WithManyCharactersToTestBufferOverflow", onEnterCallback)
    };
    
    SimpleFSM fsm;
    
    // Add states as pointer array
    State* state_ptrs[] = { &states[0], &states[1], &states[2], &states[3], &states[4], &states[5] };
    assertEqual((int)fsm.add(state_ptrs, 6), (int)FSMError::OK);
    
    // Create transitions to move through all states
    Transition transitions[] = {
        Transition(&states[0], &states[1], 1),
        Transition(&states[1], &states[2], 1),
        Transition(&states[2], &states[3], 1),
        Transition(&states[3], &states[4], 1),
        Transition(&states[4], &states[5], 1)
    };
    assertEqual((int)fsm.add(transitions, 5), (int)FSMError::OK);
    
    fsm.setInitialState(&states[0]);
    fsm.run(10);
    
    // Test getName() for each state by transitioning through them
    for (int i = 0; i < 6; i++) {
        assertEqual(fsm.getState(), &states[i]);
        
        // Verify getName() returns correct string
        String stateName = fsm.getState()->getName();
        String expected = String("State") + String(i);
        if (i == 4) expected = "LongStateName4WithSpecialChars!@#";
        if (i == 5) expected = "EvenLongerStateName5WithManyCharactersToTestBufferOverflow";
        
        assertEqual(stateName, expected);
        
        // Also test the string doesn't contain garbage characters
        for (int j = 0; j < stateName.length(); j++) {
            char c = stateName.charAt(j);
            // Verify characters are printable (not garbage)
            assertTrue(c >= 32 && c <= 126); // Printable ASCII range
        }
        
        // Move to next state if not the last one
        if (i < 5) {
            assertTrue(fsm.trigger(1));
        }
    }
    
    // Verify all states were entered correctly
    assertEqual(g_enter_count, 6);
}

// Issue #26/#27: Combined Global Transitions with String Safety
test(GitHubIssues, GlobalTransitionsWithStringSafety) {
    resetCounters();
    
    State normalState("NormalOperation", onEnterCallback);
    State emergencyState("EmergencyShutdownMode", onEnterCallback);
    State errorState("ErrorRecoveryState", onEnterCallback);
    SimpleFSM fsm;
    
    State* states[] = { &normalState, &emergencyState, &errorState };
    assertEqual((int)fsm.add(states, 3), (int)FSMError::OK);
    
    // Test global transitions with string safety
    assertEqual((int)fsm.addGlobalTransition(&emergencyState, 999), (int)FSMError::OK);
    assertEqual((int)fsm.addGlobalTransition(&errorState, 888), (int)FSMError::OK);
    
    fsm.setInitialState(&normalState);
    fsm.run(10);
    
    // Verify initial state name
    assertEqual(fsm.getState()->getName(), "NormalOperation");
    
    // Test global transition to emergency
    assertTrue(fsm.trigger(999));
    assertEqual(fsm.getState()->getName(), "EmergencyShutdownMode");
    
    // Test global transition to error from emergency state
    assertTrue(fsm.trigger(888));
    assertEqual(fsm.getState()->getName(), "ErrorRecoveryState");
    
    // Verify all names are still intact (no corruption)
    assertEqual(normalState.getName(), "NormalOperation");
    assertEqual(emergencyState.getName(), "EmergencyShutdownMode");
    assertEqual(errorState.getName(), "ErrorRecoveryState");
}

//////////////////////////////////////////////////////////////////
// Phase 8: Named State Transitions (string-based)
//////////////////////////////////////////////////////////////////

// Bug: addDOTTransition() dereferences t.to before names are resolved.
// A named-transition FSM must produce a DOT graph with the correct node
// names (and must not crash) even before the first run()/trigger().
test(NamedTransitions, DOTGenerationWithNames) {
    resetCounters();

    State standby("Standby", onEnterCallback);
    State brewing("Brewing", onEnterCallback);
    SimpleFSM fsm;

    State* states[] = { &standby, &brewing };
    assertEqual((int)fsm.add(states, 2), (int)FSMError::OK);

    Transition trans[] = { Transition("Standby", "Brewing", 1) };
    assertEqual((int)fsm.add(trans, 1), (int)FSMError::OK);
    fsm.setInitialState(&standby);

    String dot = fsm.getDotDefinition();
    assertTrue(dot.indexOf("Standby") >= 0);
    assertTrue(dot.indexOf("Brewing") >= 0);
    // The from-node must be "Standby", never mislabelled "GLOBAL".
    assertTrue(dot.indexOf("GLOBAL") < 0);
}

// Bug: isDuplicate() compares NULL from/to pointers, so two DIFFERENT named
// transitions that share an event id are wrongly treated as duplicates and
// the second is silently dropped.
test(NamedTransitions, DistinctSharedEventNotDeduped) {
    resetCounters();

    State a("A", onEnterCallback);
    State b("B", onEnterCallback);
    State c("C", onEnterCallback);
    SimpleFSM fsm;

    State* states[] = { &a, &b, &c };
    assertEqual((int)fsm.add(states, 3), (int)FSMError::OK);

    // Same event id (1), different source/target states.
    Transition trans[] = {
        Transition("A", "B", 1),
        Transition("B", "C", 1),
    };
    assertEqual((int)fsm.add(trans, 2), (int)FSMError::OK);
    assertEqual(fsm.getTransitionCount(), 2);   // neither must be dropped

    fsm.setInitialState(&a);
    fsm.run(10);
    assertTrue(fsm.trigger(1));                  // A -> B
    assertEqual(fsm.getState(), &b);
    assertTrue(fsm.trigger(1));                  // B -> C
    assertEqual(fsm.getState(), &c);
}

// Bug: an unknown fromStateName resolves to NULL, which the FSM treats as a
// global transition firing from ANY state. A typo'd source must instead make
// the transition inert, not fire from an unrelated state.
test(NamedTransitions, UnknownFromNameIsNotGlobal) {
    resetCounters();

    State a("A", onEnterCallback);
    State b("B", onEnterCallback);
    State other("Other", onEnterCallback);
    SimpleFSM fsm;

    State* states[] = { &a, &b, &other };
    assertEqual((int)fsm.add(states, 3), (int)FSMError::OK);

    // "Aa" does not exist among the states.
    Transition trans[] = { Transition("Aa", "B", 1) };
    assertEqual((int)fsm.add(trans, 1), (int)FSMError::OK);

    fsm.setInitialState(&other);
    fsm.run(10);
    assertEqual(fsm.getState(), &other);

    assertFalse(fsm.trigger(1));                 // must NOT fire from "Other"
    assertEqual(fsm.getState(), &other);
}

//////////////////////////////////////////////////////////////////
// Phase 9: API Coverage (previously untested public surface)
//////////////////////////////////////////////////////////////////

// Guards against regression of the CHANGELOG-documented constructor bug:
// "SimpleFSM() call had no effect in parameterized constructor."
test(APICoverage, ParameterizedConstructor) {
    resetCounters();

    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    SimpleFSM fsm(&state1);   // initial state via constructor

    State* states[] = { &state1, &state2 };
    assertEqual((int)fsm.add(states, 2), (int)FSMError::OK);

    // Before the first run/trigger, getState() reflects the initial state.
    assertEqual(fsm.getState(), &state1);

    fsm.run(10);
    assertEqual(fsm.getState(), &state1);
    assertEqual(g_enter_count, 1);   // initial state entered on first run
}

// getLastTransition() must be NULL before any transition and point at the
// executed transition afterwards.
test(APICoverage, GetLastTransition) {
    resetCounters();

    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    SimpleFSM fsm;

    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    fsm.setInitialState(&state1);

    Transition trans(&state1, &state2, 1, NULL, "go");
    fsm.add(&trans, 1);
    fsm.run(10);

    assertTrue(fsm.getLastTransition() == NULL);   // nothing fired yet

    assertTrue(fsm.trigger(1));
    AbstractTransition* last = fsm.getLastTransition();
    assertTrue(last != NULL);
    assertEqual(last->getName(), "go");
}

// lastTransitioned() must reset at a transition and then grow with time.
test(APICoverage, LastTransitioned) {
    resetCounters();

    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    SimpleFSM fsm;

    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    fsm.setInitialState(&state1);

    Transition trans(&state1, &state2, 1);
    fsm.add(&trans, 1);
    fsm.run(10);
    fsm.trigger(1);

    unsigned long t1 = fsm.lastTransitioned();
    delay(20);
    unsigned long t2 = fsm.lastTransitioned();
    assertTrue(t2 >= t1);
    assertTrue(t2 >= 15);   // roughly the elapsed delay
}

// State setters must be equivalent to the constructor arguments.
test(APICoverage, StateSetters) {
    resetCounters();

    State state1("Tmp", NULL);          // no callbacks yet
    state1.setName("Start");
    state1.setOnEnterHandler(onEnterCallback);
    state1.setOnStateHandler(onStateCallback);
    state1.setOnExitHandler(onExitCallback);

    State state2("State2", onEnterCallback);
    SimpleFSM fsm;

    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    fsm.setInitialState(&state1);

    assertEqual(state1.getName(), "Start");

    fsm.run(10);
    assertEqual(g_enter_count, 1);      // setOnEnterHandler took effect

    delay(15);
    fsm.run(10);
    assertEqual(g_state_count, 1);      // setOnStateHandler took effect

    Transition trans(&state1, &state2, 1);
    fsm.add(&trans, 1);
    assertTrue(fsm.trigger(1));
    assertEqual(g_exit_count, 1);       // setOnExitHandler took effect
}

// Transition setters (name, on-run, guard) must behave like the constructor
// arguments; also exercises getEventID().
test(APICoverage, TransitionSetters) {
    resetCounters();

    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    SimpleFSM fsm;

    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    fsm.setInitialState(&state1);

    Transition trans(&state1, &state2, 7);   // no on-run / guard yet
    trans.setName("jump");
    trans.setOnRunHandler(onTransitionCallback);
    trans.setGuardCondition(guardCondition);
    assertEqual(trans.getEventID(), 7);

    fsm.add(&trans, 1);
    fsm.run(10);

    // Guard blocks the transition.
    g_guard_result = false;
    assertFalse(fsm.trigger(7));
    assertEqual(fsm.getState(), &state1);
    assertEqual(g_transition_count, 0);   // on-run must not fire when blocked

    // Guard allows the transition.
    g_guard_result = true;
    assertTrue(fsm.trigger(7));
    assertEqual(fsm.getState(), &state2);
    assertEqual(g_transition_count, 1);   // on-run fired
}

// getInterval() accessor and TimedTransition setters.
test(APICoverage, TimedTransitionGettersAndSetters) {
    resetCounters();

    State state1("State1", onEnterCallback);
    State state2("State2", onEnterCallback);
    SimpleFSM fsm;

    State* states[] = { &state1, &state2 };
    fsm.add(states, 2);
    fsm.setInitialState(&state1);

    TimedTransition timed(&state1, &state2, 20);
    timed.setName("timeout");
    timed.setOnRunHandler(onTimerCallback);
    assertEqual(timed.getInterval(), 20);

    fsm.add(&timed, 1);

    bool transitioned = false;
    for (int i = 0; i < 50 && !transitioned; i++) {
        fsm.run(5);
        if (fsm.getState() == &state2) transitioned = true;
        delay(5);
    }
    assertTrue(transitioned);
    assertEqual(g_timer_count, 1);        // setOnRunHandler took effect
}

// A duplicate global timed transition must be rejected.
test(APICoverage, DuplicateGlobalTimedTransition) {
    resetCounters();

    State idle("Idle", onEnterCallback);
    State target("Target", onEnterCallback);
    SimpleFSM fsm;

    State* states[] = { &idle, &target };
    fsm.add(states, 2);
    fsm.setInitialState(&idle);

    assertEqual((int)fsm.addGlobalTimedTransition(&target, 50), (int)FSMError::OK);
    // Identical global timed transition — must be rejected, not duplicated.
    assertEqual((int)fsm.addGlobalTimedTransition(&target, 50), (int)FSMError::INVALID_PARAMETER);
    assertEqual(fsm.getTimedTransitionCount(), 1);
}

// millis() rollover: isTimeForRun() must use wrap-safe subtraction so the run
// loop keeps firing correctly across the ~49-day unsigned-long wraparound.
test(APICoverage, RunTimingHandlesMillisRollover) {
    resetCounters();

    State state1("State1", onEnterCallback);
    SimpleFSM fsm;
    State* states[] = { &state1 };
    fsm.add(states, 1);
    fsm.setInitialState(&state1);

    const unsigned long MAX = (unsigned long)-1;

    // last_run near the max; interval not yet elapsed across the wrap.
    FSMTestHelper::setLastRun(fsm, MAX - 5);
    // now = MAX - 2 → only 3 elapsed, interval 10 → must be false.
    assertFalse(FSMTestHelper::isTimeForRun(fsm, MAX - 2, 10));

    // Enough time has elapsed across the wrap → must be true.
    // now = 2 → elapsed = 8 (mod 2^width), interval 3 → must be true.
    assertTrue(FSMTestHelper::isTimeForRun(fsm, 2, 3));

    // Sanity: normal (non-wrapping) case still works.
    FSMTestHelper::setLastRun(fsm, 1000);
    assertFalse(FSMTestHelper::isTimeForRun(fsm, 1005, 10));
    assertTrue(FSMTestHelper::isTimeForRun(fsm, 1015, 10));
}

//////////////////////////////////////////////////////////////////
// Setup and Loop
//////////////////////////////////////////////////////////////////

void setup() {
    Serial.begin(115200);
    Serial.println(F("SimpleFSM Comprehensive Test Suite"));
    Serial.println(F("Testing all FSM functionality..."));
}

void loop() {
    TestRunner::run();
}