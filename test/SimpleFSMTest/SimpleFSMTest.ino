// --- Arduino required setup/loop for AUnit ---


#include <AUnitVerbose.h>
#include <SimpleFSM.h>
#include <FSMTestHelper.h>

using namespace aunit;

#define TIMEOUT_GO_TO_SLEEP 1000

// --- Mock callback and guard functions ---
static int onEnterCount = 0;
static int onExitCount = 0;
static int onTransitionCount = 0;
static int onStateCount = 0;
static bool guardValue = true;

void onEnter() { onEnterCount++; }
void onExit() { onExitCount++; }
void onTransition() { onTransitionCount++; }
void onState() { onStateCount++; }
bool guardTrue() { return true; }
bool guardFalse() { return false; }
bool guardDynamic() { return guardValue; }

// --- State callbacks for issue 25 ---
static int state1_count = 0;
static int state2_count = 0;
static int state4_count = 0;
void on_state1() { state1_count++; }
void on_state2() { state2_count++; }
void on_state4() { state4_count++; }

// --- States ---
State s1("S1", on_state1);
State s2("S2", on_state2);
State s4("S4", on_state4);
State s3("S3", onEnter, onState, onExit, true); // final state

// --- Timed transition under test ---
TimedTransition timedTransitions[] = {
  TimedTransition(&s1, &s4, TIMEOUT_GO_TO_SLEEP)
};

// --- Test Triggers ---
enum { EVT_1 = 1, EVT_2 = 2 };

SimpleFSM fsm;

// --- Test ---
test(Issue25, TimedTransitionResetsOnReentry) {
  // Setup
  state1_count = state2_count = state4_count = 0;
  fsm = SimpleFSM();
  fsm.add(timedTransitions, 1);
  fsm.setInitialState(&s1);

  // Simulate entering S1 at t=0
  fsm.run(0);
  assertEqual(fsm.getState(), &s1);

  // Simulate leaving S1 and coming back after 200ms
  FSMTestHelper::changeToState(fsm, &s2, millis());
  FSMTestHelper::changeToState(fsm, &s1, millis() + 200);

  // Wait less than TIMEOUT_GO_TO_SLEEP (should NOT transition)
  delay(800);
  fsm.run(0);
  assertEqual(fsm.getState(), &s1);

  // Wait enough to reach the timeout (should transition)
  delay(300);
  fsm.run(0);
  assertEqual(fsm.getState(), &s4);
}


// --- Test Setup ---
void setupFSM(SimpleFSM& fsm) {
  onEnterCount = onExitCount = onTransitionCount = onStateCount = 0;
  guardValue = true;
  fsm.reset();
}

// --- Test 1: Initial State ---
test(SimpleFSM, InitialState) {
  SimpleFSM fsm(&s1);
  assertEqual(fsm.getState(), &s1);
}

// --- Test 2: State Transition by Event ---
test(SimpleFSM, StateTransitionByEvent) {
  SimpleFSM fsm(&s1);
  Transition transitions[] = { Transition(&s1, &s2, EVT_1) };
  fsm.add(transitions, 1);
  fsm.trigger(EVT_1);
  assertEqual(fsm.getState(), &s2);
}

// --- Test 3: Timed Transition ---
test(SimpleFSM, TimedTransition) {
  SimpleFSM fsm(&s1);
  TimedTransition timed[] = { TimedTransition(&s1, &s2, 10) };
  fsm.add(timed, 1);
  fsm.run(0); // start
  delay(15);
  fsm.run(0);
  assertEqual(fsm.getState(), &s2);
}

// --- Test 4: Guard Condition ---
test(SimpleFSM, GuardCondition) {
  SimpleFSM fsm(&s1);
  Transition transitions[] = { Transition(&s1, &s2, EVT_1, NULL, "", guardFalse) };
  fsm.add(transitions, 1);
  fsm.trigger(EVT_1);
  assertEqual(fsm.getState(), &s1); // should not transition
  transitions[0] = Transition(&s1, &s2, EVT_1, NULL, "", guardTrue);
  fsm.add(transitions, 1);
  fsm.trigger(EVT_1);
  assertEqual(fsm.getState(), &s2); // should transition
}

// --- Test 5: Final State ---
test(SimpleFSM, FinalState) {
  SimpleFSM fsm(&s1);
  Transition transitions[] = { Transition(&s1, &s3, EVT_1) };
  fsm.add(transitions, 1);
  fsm.trigger(EVT_1);
  assertTrue(fsm.isFinished());
}

// --- Test 6: Callback Functions ---
test(SimpleFSM, CallbackFunctions) {
  SimpleFSM fsm(&s1);
  Transition transitions[] = { Transition(&s1, &s2, EVT_1, onTransition) };
  fsm.add(transitions, 1);
  fsm.trigger(EVT_1);
  assertEqual(onEnterCount, 2); // s1 and s2
  assertEqual(onExitCount, 1);  // s1
  assertEqual(onTransitionCount, 1);
}

// --- Test 7: Duplicate Transitions ---
test(SimpleFSM, DuplicateTransitions) {
  SimpleFSM fsm(&s1);
  Transition transitions[] = { Transition(&s1, &s2, EVT_1), Transition(&s1, &s2, EVT_1) };
  fsm.add(transitions, 2);
  assertEqual(fsm.getTransitionCount(), 1);
}

// --- Test 8: Reset Function ---
test(SimpleFSM, ResetFunction) {
  SimpleFSM fsm(&s1);
  Transition transitions[] = { Transition(&s1, &s2, EVT_1) };
  fsm.add(transitions, 1);
  fsm.trigger(EVT_1);
  fsm.reset();
  assertEqual(fsm.getState(), &s1);
}

// --- Test 9: getPreviousState ---
test(SimpleFSM, GetPreviousState) {
  SimpleFSM fsm(&s1);
  Transition transitions[] = { Transition(&s1, &s2, EVT_1) };
  fsm.add(transitions, 1);
  fsm.trigger(EVT_1);
  assertEqual(fsm.getPreviousState(), &s1);
}

// --- Test 10: isInState ---
test(SimpleFSM, IsInState) {
  SimpleFSM fsm(&s1);
  assertTrue(fsm.isInState(&s1));
  Transition transitions[] = { Transition(&s1, &s2, EVT_1) };
  fsm.add(transitions, 1);
  fsm.trigger(EVT_1);
  assertTrue(fsm.isInState(&s2));
  assertFalse(fsm.isInState(&s1));
}

// --- Test 11: Memory Safety and Bounds Checking ---
test(SimpleFSM, MemorySafetyBoundsChecking) {
  SimpleFSM fsm;
  
  // Test NULL pointer handling
  FSMError result = fsm.add((Transition*)nullptr, 1);
  assertEqual(result, FSMError::INVALID_PARAMETER);
  
  result = fsm.add((TimedTransition*)nullptr, 1);
  assertEqual(result, FSMError::INVALID_PARAMETER);
  
  result = fsm.add((State**)nullptr, 1);
  assertEqual(result, FSMError::INVALID_PARAMETER);
  
  // Test invalid size
  Transition validTransition(&s1, &s2, EVT_1);
  result = fsm.add(&validTransition, 0);
  assertEqual(result, FSMError::INVALID_PARAMETER);
  
  result = fsm.add(&validTransition, -1);
  assertEqual(result, FSMError::INVALID_PARAMETER);
  
  // Test successful addition
  result = fsm.add(&validTransition, 1);
  assertEqual(result, FSMError::OK);
  assertFalse(fsm.hasError());
}

// --- Test 12: Error State Management ---
test(SimpleFSM, ErrorStateManagement) {
  SimpleFSM fsm;
  
  // Initially no error
  assertEqual(fsm.getLastError(), FSMError::OK);
  assertFalse(fsm.hasError());
  
  // Trigger an error
  fsm.add((Transition*)nullptr, 1);
  assertTrue(fsm.hasError());
  assertEqual(fsm.getLastError(), FSMError::INVALID_PARAMETER);
  
  // Error string should be meaningful
  const char* errorStr = fsm.getErrorString(FSMError::INVALID_PARAMETER);
  assertTrue(strlen(errorStr) > 0);
  
  // Reset should clear error
  fsm.reset();
  assertEqual(fsm.getLastError(), FSMError::OK);
  assertFalse(fsm.hasError());
}

// --- Test 13: Global Transitions (Event-Based) ---
test(SimpleFSM, GlobalTransitions) {
  setupFSM(fsm);
  fsm.setInitialState(&s1);
  
  // Create a global transition that works from any state
  enum { EMERGENCY_EVENT = 10 };
  Transition globalTransition(NULL, &s3, EMERGENCY_EVENT);  // NULL means "from any state"
  
  // Add states and global transition
  State* testStates[] = { &s1, &s2, &s3 };
  for (int i = 0; i < 3; i++) fsm.addUniqueState(testStates[i]);
  fsm.add(&globalTransition, 1);
  
  // Test global transition from s1
  assertEqual(fsm.getState(), &s1);
  assertTrue(fsm.trigger(EMERGENCY_EVENT));
  assertEqual(fsm.getState(), &s3);
  
  // Reset and test from s2
  fsm.reset();
  fsm.setInitialState(&s2);
  assertEqual(fsm.getState(), &s2);
  assertTrue(fsm.trigger(EMERGENCY_EVENT));
  assertEqual(fsm.getState(), &s3);
  
  // Test invalid event
  fsm.reset();
  fsm.setInitialState(&s1);
  assertFalse(fsm.trigger(999));  // Non-existent event
  assertEqual(fsm.getState(), &s1);  // Should stay in current state
}

// --- Test 14: Global Timed Transitions ---
test(SimpleFSM, GlobalTimedTransitions) {
  setupFSM(fsm);
  fsm.setInitialState(&s1);
  
  // Create a global timed transition that works from any state
  TimedTransition globalTimedTransition(NULL, &s3, 500);  // NULL means "from any state"
  
  // Add states and global timed transition
  State* testStates[] = { &s1, &s2, &s3 };
  for (int i = 0; i < 3; i++) fsm.addUniqueState(testStates[i]);
  fsm.add(&globalTimedTransition, 1);
  
  // Test global timed transition from s1
  assertEqual(fsm.getState(), &s1);
  
  // Run for less than timeout - should stay in s1
  for (int i = 0; i < 10; i++) {
    fsm.run();
    delay(30);  // Total ~300ms
  }
  assertEqual(fsm.getState(), &s1);
  
  // Run for more than timeout - should transition to s3
  delay(300);  // Total now ~600ms > 500ms timeout
  fsm.run();
  assertEqual(fsm.getState(), &s3);
}

// --- Test 15: Mixed Global and Specific Transitions ---
test(SimpleFSM, MixedGlobalAndSpecificTransitions) {
  setupFSM(fsm);
  fsm.setInitialState(&s1);
  
  enum { SPECIFIC_EVENT = 20, GLOBAL_EVENT = 21 };
  
  // Create both specific and global transitions
  Transition specificTransition(&s1, &s2, SPECIFIC_EVENT);
  Transition globalTransition(NULL, &s3, GLOBAL_EVENT);
  
  Transition transitions[] = { specificTransition, globalTransition };
  
  // Add states and transitions
  State* testStates[] = { &s1, &s2, &s3 };
  for (int i = 0; i < 3; i++) fsm.addUniqueState(testStates[i]);
  fsm.add(transitions, 2);
  
  // Test specific transition from s1
  assertEqual(fsm.getState(), &s1);
  assertTrue(fsm.trigger(SPECIFIC_EVENT));
  assertEqual(fsm.getState(), &s2);
  
  // Test global transition from s2
  assertTrue(fsm.trigger(GLOBAL_EVENT));
  assertEqual(fsm.getState(), &s3);
  
  // Reset and test global transition from s1
  fsm.reset();
  fsm.setInitialState(&s1);
  assertTrue(fsm.trigger(GLOBAL_EVENT));
  assertEqual(fsm.getState(), &s3);
  
  // Test specific transition from wrong state (s2 -> doesn't have SPECIFIC_EVENT)
  fsm.reset();
  fsm.setInitialState(&s2);
  assertFalse(fsm.trigger(SPECIFIC_EVENT));  // Should fail
  assertEqual(fsm.getState(), &s2);  // Should stay in s2
}

// --- Test 16: Global Transition Priority ---
test(SimpleFSM, GlobalTransitionPriority) {
  setupFSM(fsm);
  fsm.setInitialState(&s1);
  
  enum { CONFLICT_EVENT = 30 };
  
  // Create both specific and global transitions with same event
  Transition specificTransition(&s1, &s2, CONFLICT_EVENT);
  Transition globalTransition(NULL, &s3, CONFLICT_EVENT);
  
  Transition transitions[] = { specificTransition, globalTransition };
  
  // Add states and transitions
  State* testStates[] = { &s1, &s2, &s3 };
  for (int i = 0; i < 3; i++) fsm.addUniqueState(testStates[i]);
  fsm.add(transitions, 2);
  
  // When both specific and global transitions match, first one found should win
  // Since we added specific first, it should take priority
  assertEqual(fsm.getState(), &s1);
  assertTrue(fsm.trigger(CONFLICT_EVENT));
  assertEqual(fsm.getState(), &s2);  // Should go to s2 (specific transition)
}

// Test global transition helper functions
test(SimpleFSM, GlobalTransitionHelpers) {
  State s1("State1");
  State s2("State2");
  State s3("State3");
  State s4("State4");
  
  SimpleFSM fsm(&s1);
  State* states[] = {&s1, &s2, &s3, &s4};
  FSMError result = fsm.add(states, 4);
  assertEqual(result, FSMError::OK);
  
  // Test addGlobalTransition without callback
  result = fsm.addGlobalTransition(&s2, 100);
  assertEqual(result, FSMError::OK);
  assertEqual(fsm.getTransitionCount(), 1);
  
  // Test addGlobalTransition with callback
  result = fsm.addGlobalTransition(&s3, 200, onTransition);
  assertEqual(result, FSMError::OK);
  assertEqual(fsm.getTransitionCount(), 2);
  
  // Test addGlobalTimedTransition without callback
  result = fsm.addGlobalTimedTransition(&s4, 1000);
  assertEqual(result, FSMError::OK);
  assertEqual(fsm.getTimedTransitionCount(), 1);
  
  // Test addGlobalTimedTransition with callback
  result = fsm.addGlobalTimedTransition(&s2, 2000, onTransition);
  assertEqual(result, FSMError::OK);
  assertEqual(fsm.getTimedTransitionCount(), 2);
  
  // Test that global transitions work from any state
  assertEqual(fsm.getCurrentState(), &s1);
  
  // Trigger global transition from s1
  assertTrue(fsm.trigger(100));
  assertEqual(fsm.getCurrentState(), &s2);
  
  // Trigger same global transition from s2
  assertTrue(fsm.trigger(100));
  assertEqual(fsm.getCurrentState(), &s2);  // Should stay in s2
  
  // Move to s3 and trigger global transition
  assertTrue(fsm.trigger(200));
  assertEqual(fsm.getCurrentState(), &s3);
  assertTrue(fsm.trigger(100));
  assertEqual(fsm.getCurrentState(), &s2);
  
  // Test error cases
  result = fsm.addGlobalTransition(nullptr, 300);
  assertEqual(result, FSMError::INVALID_PARAMETER);
  
  result = fsm.addGlobalTimedTransition(nullptr, 3000);
  assertEqual(result, FSMError::INVALID_PARAMETER);
  
  result = fsm.addGlobalTimedTransition(&s2, 0);
  assertEqual(result, FSMError::INVALID_PARAMETER);
}

// --- End of tests ---

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println(F("\n\nSimpleFSM Unit Tests"));
}

void loop() {
  fsm.run();
  TestRunner::run();
}
