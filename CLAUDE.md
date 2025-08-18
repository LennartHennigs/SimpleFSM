# SimpleFSM Library - AI Assistant Reference Guide

## Overview

SimpleFSM is an Arduino/ESP library for creating finite state machines. It's designed for embedded systems and IoT devices, providing a clean API for state management with callbacks, transitions, and timing.

**Compatibility**: This library is designed to work with both **Arduino IDE** and **PlatformIO**, maintaining compatibility with standard Arduino C++ (based on C++11/C++14) without advanced STL features or modern C++ constructs that might not be available in embedded environments.

## Target Platforms

- **Arduino**: Uno, Nano, Mega, etc.
- **ESP8266**: WiFi-enabled microcontrollers
- **ESP32**: Dual-core WiFi/Bluetooth microcontrollers
- **Compatible**: Any Arduino-compatible board

## Arduino/PlatformIO Compatibility

### Development Environment Support

- **Arduino IDE**: Full compatibility with Arduino IDE 1.x and 2.x
- **PlatformIO**: Complete support in PlatformIO framework
- **No external dependencies**: Works with standard Arduino libraries only

### C++ Language Constraints

This library is designed for Arduino's embedded C++ environment, which means:

- **No STL**: No use of `std::vector`, `std::string`, `std::unique_ptr`, etc.
- **Basic C++**: Uses C++11/C++14 features available in Arduino (constexpr, nullptr)
- **Arduino String**: Uses Arduino's `String` class instead of `std::string`
- **Manual memory management**: Uses `new`/`delete` instead of smart pointers
- **Function pointers**: Uses traditional function pointers, not `std::function`
- **No exceptions**: Error handling via return codes, not try/catch

### What This Means for AI Assistants

When generating code for this library:

- ❌ Don't suggest `std::` anything
- ❌ Don't use `auto` with complex types
- ❌ Don't suggest lambda functions
- ❌ Don't use `std::shared_ptr` or smart pointers
- ✅ Use Arduino `String` class
- ✅ Use traditional C-style arrays or simple classes
- ✅ Use function pointers: `void (*callback)()`
- ✅ Use Arduino timing functions: `millis()`, `delay()`

## Core Concepts

### 1. States

States represent discrete conditions or modes in your system.

```cpp
// Basic state with entry callback
State idle("idle", on_idle_enter);

// State with entry, ongoing, and exit callbacks
State running("running", on_start, on_running, on_stop);

// Final state (FSM stops when reached)
State finished("finished", on_finished, nullptr, nullptr, true);
```

**Key Points for AI:**

- States must be defined as **pointers** in arrays: `State* states[] = { &state1, &state2 }`
- States have optional callbacks: `on_enter`, `on_state`, `on_exit`
- States can be marked as `final` to stop the FSM
- State names should be descriptive for debugging

### 2. Transitions

Two types of transitions control state changes:

#### Event-Driven Transitions

```cpp
enum Events { BUTTON_PRESS = 1, SENSOR_TRIGGER = 2 };

Transition transitions[] = {
    Transition(&idle, &running, BUTTON_PRESS),
    Transition(&running, &idle, SENSOR_TRIGGER)
};
```

#### Global Transitions

**Traditional approach** - Use `NULL` as source state:

```cpp
// Emergency stop from any state
Transition(NULL, &emergency_state, EMERGENCY_EVENT);

// Global reset to idle from any state  
Transition(NULL, &idle_state, RESET_EVENT);
```

##### NEW: Helper Functions (Recommended)

```cpp
// Much cleaner and more explicit API:
fsm.addGlobalTransition(&emergency_state, EMERGENCY_EVENT);
fsm.addGlobalTransition(&idle_state, RESET_EVENT);

// With callbacks:
fsm.addGlobalTransition(&emergency_state, EMERGENCY_EVENT, onEmergency);

// Global timed transitions:
fsm.addGlobalTimedTransition(&sleep_state, 30000);  // Auto-sleep after 30s from any state
fsm.addGlobalTimedTransition(&idle_state, 60000, onTimeout);  // With callback
```

**Advantages of Helper Functions:**

- ✅ Self-documenting code - function name explains behavior
- ✅ Better type safety - prevents accidental `NULL` usage
- ✅ Consistent with existing API pattern
- ✅ IntelliSense/autocomplete friendly
- ✅ Easier to understand and maintain

**Common Global Transition Use Cases:**

- Emergency stops and safety shutdowns
- Error handling and fault recovery
- System reset and restart functionality
- Power management (sleep mode entry)
- Timeout handling across all states

#### Timed Transitions

```cpp
TimedTransition timedTransitions[] = {
    TimedTransition(&running, &idle, 5000),  // 5 seconds
    TimedTransition(&idle, &sleep, 30000)    // 30 seconds
};
```

**Key Points for AI:**

- Event IDs should start from 1 (not 0)
- Timing is in milliseconds
- Transitions can have guard conditions and callbacks

### 3. Guard Conditions

Guards prevent transitions unless conditions are met:

```cpp
bool battery_ok() { return battery_level > 20; }

Transition protected_transition(&idle, &running, START_EVENT, nullptr, "", battery_ok);
```

## Memory Management & Safety

### Error Handling (New in v2.0)

```cpp
FSMError result = fsm.add(transitions, count);
if (result != FSMError::OK) {
    Serial.println(fsm.getErrorString(result));
    // Handle error appropriately
}
```

### Safety Limits

- `MAX_STATES`: 50 states maximum
- `MAX_TRANSITIONS`: 100 regular transitions
- `MAX_TIMED_TRANSITIONS`: 50 timed transitions

### Memory Considerations

- Dynamic allocation used internally
- Bounds checking prevents overflows
- Error codes instead of crashes
- Safe for production embedded use

## Common Patterns

### 1. Simple State Machine

**See**: `examples/SimpleTransitions/SimpleTransitions.ino`

- Basic event-driven transitions
- Setup and loop patterns
- Error handling example

### 2. Timeout-Based State Machine  

**See**: `examples/TimedTransitions/TimedTransitions.ino`

- Automatic timed transitions
- Traffic light simulation
- Continuous state operations

### 3. Mixed Event and Time Transitions

**See**: `examples/MixedTransitions/MixedTransitions.ino`

- Combines regular and timed transitions
- Button interrupts with timeouts
- Complex state interactions

### 4. Guard Conditions

**See**: `examples/Guards/Guards.ino`

- Conditional transitions
- State validation logic
- Countdown timer with conditions

### 5. Many-to-One Transitions (Helper Utility)

**See**: `examples/ManyToOneTransitionExample/ManyToOneTransitionExample.ino`

- FSMUtils helper functions
- Multiple states to single target
- Screen/menu navigation patterns

### 6. Hardware Integration

**See**: `examples/SimpleTransitionWithButton/SimpleTransitionWithButton.ino`

- Physical button handling
- Debouncing considerations
- Real-world input processing

## API Reference

### Core Methods

```cpp
// Setup
SimpleFSM fsm;
SimpleFSM fsm(&initial_state);

// Adding components (returns FSMError)
FSMError add(Transition t[], int size);
FSMError add(TimedTransition t[], int size);
FSMError add(State* states[], int size);

// Control
bool trigger(int event_id);
void run(int interval = 1000, CallbackFunction tick_cb = NULL);
void reset();

// Status
State* getState() const;
State* getPreviousState() const;
bool isInState(State* state) const;
bool isFinished() const;
unsigned long lastTransitioned() const;

// Error handling
FSMError getLastError() const;
bool hasError() const;
const char* getErrorString(FSMError error) const;
```

### Callbacks

```cpp
typedef void (*CallbackFunction)();
typedef bool (*GuardCondition)();

// State callbacks
void on_enter();     // Called when entering state
void on_state();     // Called periodically while in state
void on_exit();      // Called when leaving state

// Transition callbacks
void on_transition(); // Called during transition

// FSM callbacks
fsm.setTransitionHandler(global_transition_callback);
fsm.setFinishedHandler(fsm_finished_callback);
```

## Common Issues & Solutions

### 1. Memory Issues

**Problem**: Out of memory errors
**Solution**: Use error handling, check limits, prefer static allocation

### 2. State Pointer Issues

**Problem**: States not working correctly
**Solution**: Always use `State*[]` arrays, not `State[]`

### 3. Timer Issues

**Problem**: Timed transitions not working
**Solution**: Call `fsm.run()` regularly in loop(), check timer reset bug fix

### 4. Event ID Issues

**Problem**: Events not triggering
**Solution**: Use enum starting from 1, not 0

## Arduino-Specific Considerations

### Setup Pattern

```cpp
void setup() {
    Serial.begin(115200);
    
    // Initialize FSM with error checking
    if (fsm.add(transitions, count) != FSMError::OK) {
        Serial.println("FSM setup failed!");
        return;
    }
    
    fsm.setInitialState(&initial_state);
}

void loop() {
    fsm.run();  // Must be called regularly
    
    // Handle inputs
    if (digitalRead(BUTTON_PIN) == HIGH) {
        fsm.trigger(BUTTON_EVENT);
    }
}
```

### Serial Debugging

```cpp
// Enable debugging
void on_state_change() {
    Serial.print("State: ");
    Serial.println(fsm.getState()->getName());
}

fsm.setTransitionHandler(on_state_change);
```

### Non-blocking Operation

```cpp
void loop() {
    fsm.run(100);  // Run every 100ms
    
    // Other non-blocking code
    handle_sensors();
    handle_network();
}
```

## Testing Recommendations

### Unit Testing

```cpp
#include "FSMTestHelper.h"  // For testing only

// Direct state change for testing
FSMTestHelper::changeToState(fsm, &test_state);
```

### Integration Testing

- Test all state transitions
- Verify timeout behaviors
- Check error conditions
- Test memory limits

## Performance Notes

### Timing Characteristics

- State transitions: ~50μs typical
- Timer checking: ~10μs per timed transition
- Memory allocation: Avoid in loop()

### Memory Usage

- Base FSM: ~200 bytes
- Per state: ~50 bytes
- Per transition: ~30 bytes
- Dynamic allocation for growth

## Version Compatibility

### Breaking Changes in v2.0

- States must be pointer arrays: `State*[]`
- Add methods return `FSMError`
- Constructor fix
- New safety limits

### Migration from v1.x

```cpp
// Old (v1.x)
State states[] = { State("name", callback) };
fsm.add(states, 1);

// New (v2.x)
State states[] = { State("name", callback) };
State* state_ptrs[] = { &states[0] };
FSMError result = fsm.add(state_ptrs, 1);
```

## Best Practices for AI Assistants

### Arduino/PlatformIO Compliance

1. **Stick to Arduino C++** - No STL, no modern C++ features beyond C++11/14 basics
2. **Use Arduino libraries** - `String`, `Serial`, `millis()`, etc.
3. **Traditional patterns** - Function pointers, not lambdas; arrays, not vectors
4. **Memory awareness** - Consider embedded constraints

### FSM Implementation

1. **Always check error codes** when adding components
2. **Use pointer arrays** for states: `State*[]`
3. **Start event IDs from 1**, not 0
4. **Call `fsm.run()`** regularly in loop
5. **Use meaningful state/transition names** for debugging
6. **Implement proper error handling** for production code
7. **Consider memory limits** when designing large FSMs
8. **Test state machines thoroughly** with all edge cases

## Debugging Tips

### Common Debug Patterns

```cpp
// State change logging
void debug_transition() {
    Serial.print(fsm.getPreviousState()->getName());
    Serial.print(" -> ");
    Serial.println(fsm.getState()->getName());
}

// Error checking
if (fsm.hasError()) {
    Serial.print("FSM Error: ");
    Serial.println(fsm.getErrorString(fsm.getLastError()));
}

// GraphViz visualization
Serial.println(fsm.getDotDefinition());
```

This reference should help AI assistants understand and work with the SimpleFSM library effectively, considering its Arduino/embedded context and specific requirements.
