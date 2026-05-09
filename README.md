# SimpleFSM

Arduino/ESP library to simplify setting up and running a state machine.

* Author: Lennart Hennigs (<https://www.lennarthennigs.de>)
* Copyright (C) 2022-2026 Lennart Hennigs.
* Released under the MIT license.

## Description

This library allows you to quickly setup a State Machine. Read here [what a state machine is](https://majenko.co.uk/blog/our-blog-1/the-finite-state-machine-26) and [why a state machine is neat for hardware projects](https://barrgroup.com/embedded-systems/how-to/state-machines-event-driven-systems?utm_source=pocket_mylist).

It has been tested with Arduino, ESP8266 and ESP32 devices.

⚠️ To see the latest changes to the library please take a look at the [Changelog](https://github.com/LennartHennigs/SimpleFSM/blob/main/CHANGELOG.md).

If you find this library helpful please consider giving it a ⭐️ at [GitHub](https://github.com/LennartHennigs/SimpleFSM) and/or [buy me a ☕️](https://ko-fi.com/lennart0815). Thanks!

### Features

* Easy state definition
* Allows for none, one or multiple final states
* Event triggered transitions and (automatic) timed transitions
* Possibility to define guard conditions for your transitions
* Callback functions for...
  * State entry, staying, exit
  * Transition execution
  * Final state reached
  * the run interval of the state machine
* Definition of an in_state interval
* Functions for tracking the behavior and progress of the state machine
* Creation of a `Graphviz` source file of your state machine definition
  
## How To Use

* You first need to define a set of states for your state machine
* Then, define transitions (regular or timed) for these state
* Pass the transitions to your state machine
* Define an initial state
* ...and add the `run()` function in your loop
* See [SimpleTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/SimpleTransitions/SimpleTransitions.ino) for a basic example

### Defining States

* A finite state machine has a defined set of states
* A state must have a name...
* ...and can have callback functions for different events (when a state is entered, exited, or stays in a state)
* Always define your `State` objects outside of functions (globally or as static variables) to ensure they remain valid for the lifetime of the FSM.

```c++
  State(
    String name, 
    CallbackFunction on_enter, 
    CallbackFunction on_state = NULL, 
    CallbackFunction on_exit = NULL, 
    bool is_final = false
  );
```

* Most states will have the entry handler
* The easiest way to define states is creating using an array, e.g. as shown in [MixedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/MixedTransitions/MixedTransitions.ino):
  
  ```c++
  State s[] = {
    State("red",        on_red),
    State("green",      on_green),
    State("BTN",        on_button_press)
  };
  ```

* The initial state **must** of the state machine must be defined – either via the constructor or the `setup()` function or via the `setInitialState()` function
* None, one, or multiple states can be defined as an end state via `setAsFinal()`
* For the full `State` class definition see [State.h](https://github.com/LennartHennigs/SimpleFSM/blob/main/src/State.h)

### Transitions

* This library offers two types of Transitions, regular and timed ones
* All transitions must have a from and and a to state
* Transitions can have a callback function for when the transition is executed, a name, and a [guard condition](#guard-conditions)
* You can add both types to a state machine, see [MixedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/MixedTransitions/MixedTransitions.ino) for an example
* See [Transitions.h](https://github.com/LennartHennigs/SimpleFSM/blob/main/src/Transitions.h) for the class definitions of `Transition` and `TimedTransition`.
* Note: Both classes are based of an abstract class which is not to be used in your code.

### Regular Transitions

```c++
  Transition(
    State* from, 
    State* to, 
    int event_id, 
    CallbackFunction on_run = NULL, 
    String name = "", 
    GuardCondition guard = NULL
  );
```

* Regular transitions must have a a set of states and a trigger (ID):

  ```c++
  Transition transitions[] = {
    Transition(&s[0], &s[1], light_switch_flipped),
    Transition(&s[1], &s[0], light_switch_flipped)
  };
  ```

* Define the triggers for your state machine in an enum (and set the first enum value to 1, to be safe):

  ```c++
  enum triggers {
    light_switch_flipped = 1  
  };
  ```

* To call a trigger use the `trigger()`function:

  ```c++
  fsm.trigger(light_switch_flipped);
  ```

### Global Transitions

Global transitions allow you to define transitions that work from **any state**. You can use either the traditional approach or the new helper functions. They are internally defined as states with a "NULL" source state. Use the helper function to define them


```c++
// Much cleaner and more explicit API:
fsm.addGlobalTransition(&error_state, EMERGENCY_EVENT);
fsm.addGlobalTransition(&idle, RESET_EVENT);

// With callbacks:
fsm.addGlobalTransition(&error_state, EMERGENCY_EVENT, onEmergency);

// Global timed transitions:
fsm.addGlobalTimedTransition(&sleep_state, 30000);  // Auto-sleep after 30s from any state
```

**Use Cases:**
* **Emergency stops**: Transition to a safe state from anywhere
* **Error handling**: Jump to error state when problems occur
* **System reset**: Return to initial state from any point
* **Power management**: Enter sleep mode from any state

**Multiple Global Transitions:**

When you have multiple global transitions:
* **Same event ID**: Only the **first** matching transition will execute (order matters!)
* **Different event IDs**: Each can be triggered independently  
* **Priority**: Transitions are checked in the order they were added to the FSM
* **Global vs Specific**: If both exist for the same event, the first one added takes priority

⚠️ **Important**: If you add multiple global transitions with the same event ID, only the first one will be triggered. The system will detect this situation but won't prevent it for backward compatibility.

**Example with Helper Functions:**

```c++
// Define states
State idle("Idle");
State running("Running");
State processing("Processing");
State error_state("Error");

// Add states to FSM
State* states[] = {&idle, &running, &processing, &error_state};
fsm.add(states, 4);

// Regular transitions (between specific states)
Transition regular_transitions[] = {
    Transition(&idle, &running, START_EVENT),
    Transition(&running, &processing, PROCESS_EVENT)
};
fsm.add(regular_transitions, 2);

// Global transitions using helper functions (RECOMMENDED)
fsm.addGlobalTransition(&error_state, ERROR_EVENT);    // Any state -> Error
```

* See [GlobalTransitionsSimple.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/GlobalTransitionsSimple/GlobalTransitionsSimple.ino) and [GlobalTransitionHelpers.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/GlobalTransitionHelpers/GlobalTransitionHelpers.ino) for more details

### Named State Transitions

Instead of passing `State*` pointers, you can define transitions using state **names** (strings). Names are resolved automatically on the first `run()` or `trigger()` call, so transitions and states can be declared in any order:

```cpp
// Transitions declared by name — no State* pointers needed at this point
Transition transitions[] = {
  Transition("Standby", "Brewing", BREW_BUTTON),
  Transition("Ready",   "Standby", COFFEE_TAKEN),
};
TimedTransition timedTransitions[] = {
  TimedTransition("Brewing", "Ready", BREW_TIME_MS),
};

// States defined afterwards — order doesn't matter
State standby("Standby", onEnterStandby);
State brewing("Brewing", onEnterBrewing);
State ready  ("Ready",   onEnterReady);
```

* See [NamedStateTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/NamedStateTransitions/NamedStateTransitions.ino) for a complete example

### Timed Transitions

```c++
  TimedTransition(
    State* from, 
    State* to, 
    int interval, 
    CallbackFunction on_run = NULL, 
    String name = "", 
    GuardCondition guard = NULL
  );
```

* Timed transitions are automatically executed after a certain amount of time has passed
* The interval is defined in milliseconds:

  ```c++
  TimedTransition timedTransitions[] = {
    TimedTransition(states[0], states[1], 6000),
    TimedTransition(states[1], states[0], 4000),
    TimedTransition(states[2], states[1], 2000)
  };
  ```

* See [TimedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/TimedTransitions/TimedTransitions.ino) for more details

### Guard Conditions

* Guard conditions are evaluated before a transition takes places
* Only if the guard condition is true the transition will be executed
* Both regular and timed transitions can have guard conditions
* You define guard conditions as functions:

  ```c++
  TimedTransition timedTransitions[] = {
    TimedTransition(states[0], states[1], 1000, NULL, "", zero_yet),
    TimedTransition(states[0], states[0], 1000, NULL, "", not_zero_yet)
  };

  bool not_zero_yet() {
    return countdown != 0;
  }

  bool zero_yet() {
    return countdown == 0;
  }
  ```

* See [Guards.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/Guards/Guards.ino) for a complete example

### In-State Interval

* States can have up to three events...
  * when they enter a state
  * while they are in a state
  * ...and when they leave the state

```c++
  State(String name, CallbackFunction on_enter, CallbackFunction on_state = NULL, CallbackFunction on_exit = NULL, bool is_final = false);
```

* To define how frequent the `on_state` event is called, pass an interval (in ms) to the `run()` function in your main `loop()`:

  ```c++
  void run(int interval = 1000, CallbackFunction tick_cb = NULL);
  ```

* After this interval has passed the `on_state` function of the active state will be called (if it defined)
* Also the `tick_cb`callback function will be called (if defined) in the `run()` call

### Helper functions

* SimpleFSM provides a few functions to check on the state of the machine:

  ```c++
    State* getState();
    State* getPreviousState() const;
    bool isInState(State* state) const;
    unsigned long lastTransitioned() const;
    bool isFinished() const;

  ```

### GraphViz Generation

* Use the function `getDotDefinition()` to get your state machine definition in the GraphViz [dot format](https://www.graphviz.org/doc/info/lang.html)
* Here the output for the [MixedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/MixedTransitions/MixedTransitions.ino) example:
  
  ```c++
    digraph G {
      rankdir=LR; pad=0.5
      node [shape=circle fixedsize=true width=1.5];
      "red light" -> "green light" [label=" (6000ms)"];
      "green light" -> "red light" [label=" (4000ms)"];
      "button pressed" -> "green light" [label=" (2000ms)"];
      "red light" -> "button pressed" [label=" (ID=1)"];
      "red light" [style=filled fontcolor=white fillcolor=black];
    }
  ```

* You can use this visualize your state machine:\
  ![MixedTransitions.ino example](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/MixedTransitions/MixedTransitions.svg?raw=true)

* If the machine is running, the current state will be highlighted
* Currently guard functions and end states are not shown in the graph
* See [MixedTransitionsBrowser.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/MixedTransitionsBrowser/MixedTransitionsBrowser.ino) to learn how to run a webserver to show the Graphviz diagram of your state machine

### Utility Helpers for Many-to-One Transitions

To easily create transitions from multiple source states to a single target state, use the helpers in `FSMUtils.h`:

```cpp
#include "FSMUtils.h"

// Regular transitions
FSMUtils::createManyToOneTransitions(sources, num_sources, target, event_id, out_array);

// Timed transitions
FSMUtils::createManyToOneTimedTransitions(sources, num_sources, target, interval, out_array);
```

See `ManyToOneTransitionExample.ino` for a complete usage example.

## Class Definitions

* [State.h](https://github.com/LennartHennigs/SimpleFSM/blob/main/src/State.h)
* [Transitions.h](https://github.com/LennartHennigs/SimpleFSM/blob/main/src/Transitions.h) for the class definition of both transitions
* [SimpleFSM](https://github.com/LennartHennigs/SimpleFSM/blob/main/src/SimpleFSM.h)
* [FSMUtils.h](https://github.com/LennartHennigs/SimpleFSM/blob/main/src/FSMUtils.h) - utility helpers for generating many-to-one transitions

## Examples

* [SimpleTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/SimpleTransitions/SimpleTransitions.ino) - only regular transitions and showcasing the different events
* [SimpleTransitionWithButtons.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/SimpleTransitionWithButton/SimpleTransitionWithButton.ino) - event is now triggered via a hardware button
* [TimedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/TimedTransitions/TimedTransitions.ino) - showcasing timed transitions
* [MixedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/MixedTransitions/MixedTransitions.ino) - regular and timed transitions
* [MixedTransitionsBrowser.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/MixedTransitionsBrowser/MixedTransitionsBrowser.ino) - creates a webserver to show the Graphviz diagram of the state machine
* [Guards.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/Guards/Guards.ino) - showing how to define guard functions
* [ManyToOneTransitionExample.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/ManyToOneTransitionExample/ManyToOneTransitionExample.ino) - demonstrates utility helpers for creating many-to-one transitions
* [GlobalTransitionsSimple.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/GlobalTransitionsSimple/GlobalTransitionsSimple.ino) - demonstrates global transitions using the addGlobalTransition() helper functions
* [GlobalTransitionHelpers.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/GlobalTransitionHelpers/GlobalTransitionHelpers.ino) - complete demonstration of global transition helper functions including timed global transitions
* [NamedStateTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/main/examples/NamedStateTransitions/NamedStateTransitions.ino) - demonstrates string-based state-name constructors for `Transition` and `TimedTransition`

## Testing

The library includes comprehensive testing infrastructure to ensure compatibility across different Arduino platforms:

### Compilation Testing

A bash script (`test/compile_examples.sh`) automatically tests compilation of all examples across multiple platforms:

* **Wemos D1 Mini** (ESP8266)
* **M5Stack Core2** (ESP32)
* **Arduino Nano** (AVR)

**Usage:**

```bash
cd test
./compile_examples.sh          # Run all tests
./compile_examples.sh --help   # Show help and options
```

**Features:**

* Automatic platform-specific example exclusion (e.g., WiFi examples skip Arduino Nano)
* Colored output with clear pass/fail indicators
* Detailed error reporting for failed compilations
* Summary statistics

**Prerequisites:**

* [arduino-cli](https://arduino.github.io/arduino-cli/) installed and in PATH
* Required Arduino cores: `esp8266:esp8266`, `esp32:esp32`, `arduino:avr`

### Unit Testing

The library includes comprehensive unit tests using the [AUnit](https://github.com/bxparks/AUnit) testing framework:

**Test Coverage:**
* FSM state transitions and callbacks
* Timed transitions and timeout handling
* Guard conditions and validation
* Error handling and memory safety
* Global transition functionality

**Dependencies:**
* **AUnit** (v1.7.1+) - Arduino unit testing framework by Brian T. Park
* Automatically installed via PlatformIO `test_deps`
* For Arduino IDE: Install manually via Library Manager

**Running Tests:**

```bash
# PlatformIO - Native tests using EpoxyDuino (no hardware required)
pio test -e epoxy-esp8266        # Emulates ESP8266
pio test -e epoxy-esp32          # Emulates ESP32

# PlatformIO - Hardware tests on specific platform
pio test -e Wemos_test           # Real ESP8266 hardware
pio test -e M5Stack_ESP32_test   # Real ESP32 hardware  
pio test -e Nano_test            # Real Arduino Nano hardware

# Arduino IDE - open test/SimpleFSMTest/SimpleFSMTest.ino
```

## Notes

* This library is heavily inspired by the [Arduino-fsm](https://github.com/jonblack/arduino-fsm) library created by [Jon Black](https://github.com/jonblack). I initially used some of his as a base. Without Jon's work this library would not exist.
* To see the latest changes to the library please take a look at the [Changelog](https://github.com/LennartHennigs/SimpleFSM/blob/main/CHANGELOG.md).
* And if you find this library helpful, please consider giving it a star at [GitHub](https://github.com/LennartHennigs/SimpleFSM). Thanks!

## Migrating from v1.x to v2.0

⚠️ **Version 2.0 introduces breaking changes** that require code modifications when upgrading from v1.x.

### Key Breaking Changes

1. **State Arrays Must Use Pointers**
2. **Add Methods Return Error Codes**
3. **New Memory Safety Limits**
4. **Constructor Behavior Fixed**

### Step-by-Step Migration Guide

#### 1. Update State Definitions

**Before (v1.x):**
```cpp
State states[] = {
    State("idle", on_idle),
    State("running", on_running)
};
fsm.add(states, 2);  // This won't work in v2.0
```

**After (v2.0):**
```cpp
State states[] = {
    State("idle", on_idle),
    State("running", on_running)
};
State* state_ptrs[] = { &states[0], &states[1] };
fsm.add(state_ptrs, 2);  // Use pointer array
```

#### 2. Add Error Handling

**Before (v1.x):**
```cpp
fsm.add(transitions, num_transitions);  // void return
```

**After (v2.0):**
```cpp
FSMError result = fsm.add(transitions, num_transitions);
if (result != FSMError::OK) {
    Serial.print("Error: ");
    Serial.println(fsm.getErrorString(result));
    // Handle error appropriately
}
```

#### 3. Check Memory Limits

v2.0 enforces safety limits:
- **MAX_STATES**: 50 states maximum
- **MAX_TRANSITIONS**: 100 regular transitions maximum
- **MAX_TIMED_TRANSITIONS**: 50 timed transitions maximum

If you exceed these limits, consider breaking your FSM into smaller components.

#### 4. Update Include Statements (if using utilities)

**New in v2.0:**
```cpp
#include "FSMUtils.h"  // For many-to-one transition helpers
```

### Migration Checklist

- [ ] Convert state arrays to use pointers: `State*[]`
- [ ] Add error checking to all `fsm.add()` calls
- [ ] Verify your FSM doesn't exceed the new safety limits
- [ ] Test all state transitions work as expected
- [ ] Update any custom test code to use new error handling

### Quick Migration Example

**Complete v1.x code:**
```cpp
State s1("state1", callback1);
State s2("state2", callback2);
State states[] = { s1, s2 };

Transition transitions[] = {
    Transition(&s1, &s2, EVENT_1)
};

void setup() {
    fsm.add(states, 2);
    fsm.add(transitions, 1);
    fsm.setInitialState(&s1);
}
```

**Migrated v2.0 code:**
```cpp
State s1("state1", callback1);
State s2("state2", callback2);
State* states[] = { &s1, &s2 };  // ← Pointer array

Transition transitions[] = {
    Transition(&s1, &s2, EVENT_1)
};

void setup() {
    // ← Add error checking
    if (fsm.add(states, 2) != FSMError::OK) {
        Serial.println("Failed to add states");
        return;
    }
    if (fsm.add(transitions, 1) != FSMError::OK) {
        Serial.println("Failed to add transitions");
        return;
    }
    fsm.setInitialState(&s1);
}
```

### Why These Changes?

- **Memory Safety**: Prevents crashes from memory overflows
- **Error Handling**: Makes debugging easier and code more robust
- **Pointer Arrays**: Fixes memory management issues and improves performance
- **Production Ready**: Makes the library suitable for production embedded systems

## How To Install

Open the Arduino IDE choose "Sketch > Include Library" and search for "SimpleFSM".
Or download the ZIP archive (<https://github.com/lennarthennigs/SimpleFSM/zipball/main>), and choose "Sketch > Include Library > Add .ZIP Library..." and select the downloaded file.

## License

MIT License

Copyright (c) 2022-2026 Lennart Hennigs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
