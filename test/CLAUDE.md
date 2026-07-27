# SimpleFSM Test Suite - AI Assistant Guidelines

This file provides specific guidance for AI assistants (especially Claude Code) when working with the SimpleFSM test suite.

## Test Suite Overview

The SimpleFSM test suite is organized into 9 comprehensive phases covering all aspects of the finite state machine library:

1. **StateManagement** - State creation, callbacks, properties, and lifecycle
2. **TransitionTests** - Event-driven transitions, guards, and validation  
3. **TimedTransitions** - Time-based automatic transitions and timing behavior
4. **GlobalTransitions** - Global transitions using both helper methods and traditional approaches
5. **ErrorHandling** - Error codes, parameter validation, and edge cases
6. **AdvancedFeatures** - FSM reset, DOT generation, state tracking, and handlers
7. **Integration** - Real-world scenarios like traffic lights and complex state machines
8. **NamedTransitions** - String-based (name) transitions: DOT generation, duplicate detection, and unresolved-name safety
9. **APICoverage** - Previously untested public surface: parameterized constructor, `getLastTransition()`, `lastTransitioned()`, setters, accessors, and `millis()`-rollover run timing

## Testing Framework & Environment

### Dependencies
- **AUnit** (v1.7.1+) - Arduino unit testing framework
- **EpoxyDuino** - For native testing without hardware (epoxy-esp8266, epoxy-esp32)
- **FSMTestHelper** - Helper class for accessing private FSM methods in tests

### Supported Test Environments
- `epoxy-esp8266` - ESP8266 emulation (recommended for fast testing)
- `epoxy-esp32` - ESP32 emulation 
- `Wemos_test` - Real ESP8266 hardware
- `M5Stack_ESP32_test` - Real ESP32 hardware
- `Nano_test` - Real Arduino Nano hardware

## Critical Testing Rules

### 1. Test Isolation (MANDATORY)
Each test MUST be completely isolated from other tests:

```cpp
test(TestSuite, TestName) {
    resetCounters(); // ALWAYS reset global counters first
    
    // Create LOCAL instances - never use global objects
    State state1("State1", onEnterCallback);
    SimpleFSM fsm; // Local FSM instance
    
    // Test logic here
}
```

**Why**: Global state pollution was the root cause of previous segmentation faults.

### 2. Arduino C++ Compatibility (MANDATORY)
Always use Arduino-compatible C++ constructs:

```cpp
// CORRECT - Use NULL for Arduino compatibility
Transition globalTrans(NULL, &target, event);
if (state == NULL) { ... }

// WRONG - nullptr may not be available in all Arduino environments
Transition globalTrans(nullptr, &target, event); // DON'T USE
if (state == nullptr) { ... } // DON'T USE
```

**Why**: Arduino uses embedded C++ which may not support all modern C++ features.

### 3. Timing in Timed Transition Tests
For tests involving `TimedTransition`, ALWAYS use small run intervals:

```cpp
// CORRECT - Use small interval for timed transitions
fsm.run(10); // 10ms interval

// WRONG - Default 1000ms interval will miss short timeouts
fsm.run(); // Uses 1000ms default, will miss 50ms timeouts
```

**Why**: The FSM only checks timed transitions when `run()` is called, and the default interval is 1000ms.

### 4. Counter Management
Use the provided callback counter system:

```cpp
// Global counters (already defined)
static int g_enter_count = 0;
static int g_state_count = 0;
static int g_exit_count = 0;
// etc.

// Always reset before each test
resetCounters();

// Use in assertions
assertEqual(g_enter_count, 1);
```

### 5. Memory Safety
- Always use **pointer arrays** for states: `State* states[] = { &state1, &state2 };`
- Never use value arrays: `State states[] = { ... }` (won't work with FSM.add())
- Check return values: `FSMError result = fsm.add(states, 2);`

### 6. Timing Loops for Timed Transitions
Use this pattern for testing timed transitions:

```cpp
bool transitioned = false;
for (int i = 0; i < MAX_ATTEMPTS && !transitioned; i++) {
    fsm.run(SMALL_INTERVAL); // e.g., 10ms
    if (fsm.getState() == &target_state) {
        transitioned = true;
    }
    delay(SMALL_DELAY); // e.g., 5ms
}
assertTrue(transitioned);
```

## Test Categories and Expectations

### StateManagement Tests
- Test all state callbacks: `on_enter`, `on_state`, `on_exit`
- Test final states and FSM termination
- Test state properties and setters
- Test state ID assignment and uniqueness

### TransitionTests  
- Test event-driven transitions with various event IDs
- Test guard conditions that allow/block transitions
- Test transition callbacks (`on_run_cb`)
- Test invalid event handling
- Test multiple transitions from same state

### TimedTransitions Tests
- **CRITICAL**: Use `fsm.run(10)` with 10ms interval for timing tests
- Test single timed transitions with callbacks
- Test multiple overlapping timed transitions
- Test timed transitions with guard conditions
- Test timer reset behavior on state changes

### GlobalTransitions Tests
- Test modern helper API: `addGlobalTransition()`, `addGlobalTimedTransition()`
- Test traditional approach: `Transition(NULL, &target, event)` (NOTE: Use NULL, not nullptr for Arduino compatibility)
- Test global transitions work from multiple states
- Test global vs local transition interactions

### ErrorHandling Tests
- Test all `FSMError` enum values
- Test parameter validation (null pointers, zero sizes)
- Test memory limits (MAX_STATES, MAX_TRANSITIONS, MAX_TIMED_TRANSITIONS)
- Test error string retrieval
- Test duplicate detection

### AdvancedFeatures Tests
- Test `fsm.reset()` returns to initial state
- Test `setTransitionHandler()` global callback
- Test state tracking: `isInState()`, `getPreviousState()`, `lastTransitioned()`
- Test DOT graph generation: `getDotDefinition()`

### Integration Tests
- Test realistic scenarios (traffic light FSM)
- Test complex FSMs with mixed transition types
- Test FSM behavior under heavy load
- Test complete FSM lifecycle from creation to termination

## Common Pitfalls to Avoid

### 1. Global State Pollution
❌ **NEVER do this**:
```cpp
// Global FSM instance - will cause test interference
SimpleFSM global_fsm;

test(Bad, Example) {
    global_fsm.add(...); // Affects other tests!
}
```

✅ **Always do this**:
```cpp
test(Good, Example) {
    resetCounters();
    SimpleFSM fsm; // Local instance
    // Test logic
}
```

### 2. Wrong Timing for Timed Transitions
❌ **NEVER do this**:
```cpp
fsm.run(); // Uses 1000ms default - will miss short timeouts
```

✅ **Always do this**:
```cpp
fsm.run(10); // Use 10ms for responsive timing tests
```

### 3. Forgetting Counter Reset
❌ **NEVER do this**:
```cpp
test(Bad, Example) {
    // No resetCounters() call
    assertEqual(g_enter_count, 1); // May fail due to previous tests
}
```

✅ **Always do this**:
```cpp
test(Good, Example) {
    resetCounters(); // ALWAYS reset first
    assertEqual(g_enter_count, 0); // Now predictable
}
```

### 4. NULL vs nullptr Compatibility Issues
❌ **NEVER do this**:
```cpp
// Don't use nullptr - not available in all Arduino environments
Transition global(nullptr, &target, RESET_EVENT);
if (fsm.getState() == nullptr) { ... }
```

✅ **Always do this**:
```cpp
// Use NULL for Arduino compatibility
Transition global(NULL, &target, RESET_EVENT);
if (fsm.getState() == NULL) { ... }
```

### 5. Using Value Arrays Instead of Pointer Arrays
❌ **NEVER do this**:
```cpp
State states[] = { State("State1", callback) };
fsm.add(states, 1); // Won't work!
```

✅ **Always do this**:
```cpp
State state1("State1", callback);
State* states[] = { &state1 };
fsm.add(states, 1); // Correct!
```

## Running Tests

### Command Examples
```bash
# Fast native testing (recommended)
pio test -e epoxy-esp8266 -v    # ESP8266 emulation (~3-5 seconds)
pio test -e epoxy-esp32 -v      # ESP32 emulation (~3-5 seconds)

# Hardware testing (slower)
pio test -e Wemos_test -v       # Real ESP8266 hardware
pio test -e M5Stack_ESP32_test -v # Real ESP32 hardware
```

### Expected Results
- **All 41 tests should PASS**
- **No segmentation faults**
- **No memory leaks**
- **Total runtime: ~5-10 seconds for native tests**

## Debugging Failed Tests

### Common Debug Patterns
```cpp
// Add debug output to failing tests
Serial.print("Current state: ");
Serial.println(fsm.getState()->getName().c_str());
Serial.print("Counter values: enter=");
Serial.print(g_enter_count);
Serial.print(" state=");
Serial.println(g_state_count);
```

### Memory Issues
- Check for null pointer dereferences
- Verify proper array sizes
- Ensure states are properly added to FSM before use
- **Helper Method Memory Allocation**: If using `addGlobalTransition()` or `addGlobalTimedTransition()`, verify proper memory allocation handling

### Helper Method Debugging
If experiencing segfaults with global transition helper methods:

```cpp
// Test helper method memory allocation
FSMError result = fsm.addGlobalTransition(&emergency_state, EMERGENCY_EVENT);
if (result != FSMError::OK) {
    Serial.print("Helper method failed: ");
    Serial.println(fsm.getErrorString(result));
}
```

**Known Issues (Fixed in v2.1)**:
- Helper methods had memory allocation bugs causing segfaults
- DOT generation methods had NULL pointer issues with global transitions  
- Memory arrays were not properly allocated during dynamic resizing

### Timing Issues  
- Increase loop attempt counts for slower platforms
- Adjust delay values for different timing requirements
- Use `Serial.print()` to trace timing behavior

## Test Maintenance Guidelines

### Adding New Tests
1. **Follow the phase organization** - add to appropriate test suite
2. **Use the established patterns** - copy existing successful test structure
3. **Always include `resetCounters()`** at the beginning
4. **Use descriptive test names** - `test(Suite, DescriptiveName)`
5. **Test both positive and negative cases**
6. **Use established callback patterns** for consistent counter tracking
7. **Check FSMError return values** for all FSM operations
8. **Use NULL instead of nullptr** for Arduino compatibility

### Established Test Patterns

#### Basic Test Structure
```cpp
test(TestSuite, DescriptiveName) {
    resetCounters(); // MANDATORY first line
    
    // Local state instances
    State state1("State1", onEnterCallback1, onStateCallback1, onExitCallback1);
    State state2("State2", onEnterCallback2);
    
    // Local FSM instance
    SimpleFSM fsm;
    
    // Add states with error checking
    State* states[] = { &state1, &state2 };
    FSMError result = fsm.add(states, 2);
    assertEqual(result, FSMError::OK);
    
    // Test logic with assertions
    assertTrue(fsm.trigger(EVENT_ID));
    assertEqual(fsm.getState(), &state2);
    assertEqual(g_enter_count, 2); // Both states entered
}
```

#### Global Transition Test Pattern
```cpp
test(GlobalTransitions, HelperMethod) {
    resetCounters();
    
    State idle("Idle", onEnterCallback1);
    State emergency("Emergency", onEnterCallback2);
    SimpleFSM fsm;
    
    // Add states and set initial
    State* states[] = { &idle, &emergency };
    assertEqual(fsm.add(states, 2), FSMError::OK);
    fsm.setInitialState(&idle);
    
    // Test helper method with error checking
    FSMError result = fsm.addGlobalTransition(&emergency, EMERGENCY_EVENT);
    assertEqual(result, FSMError::OK);
    
    // Verify global transition works
    assertTrue(fsm.trigger(EMERGENCY_EVENT));
    assertEqual(fsm.getState(), &emergency);
}
```

### Modifying Existing Tests
1. **Preserve test isolation** - don't introduce global dependencies
2. **Maintain timing patterns** - keep small intervals for timed tests
3. **Update comments** if behavior changes
4. **Run full test suite** after modifications

### Performance Considerations
- Native tests (epoxy) are ~10x faster than hardware tests
- Use hardware tests only for final validation
- Prefer shorter timeouts in tests (30-100ms) for faster execution
- Group related tests to minimize setup overhead

## Library-Specific Considerations

### Arduino/PlatformIO Compatibility
- Tests run in both Arduino and PlatformIO environments
- EpoxyDuino provides Arduino API emulation for native testing
- No external dependencies beyond AUnit and EpoxyDuino

### Memory Constraints
- ESP8266: ~80KB RAM - be mindful of large test arrays
- ESP32: ~320KB RAM - more generous but still finite
- Arduino Nano: ~2KB RAM - very limited, tests must be lean

### Timing Accuracy
- Native tests use system clock - very accurate
- Hardware tests depend on crystal accuracy - usually good enough
- Use delay() for predictable timing in tests

This comprehensive test suite ensures the SimpleFSM library is robust, reliable, and production-ready across all supported Arduino platforms.