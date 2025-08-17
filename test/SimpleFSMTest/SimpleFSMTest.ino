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
