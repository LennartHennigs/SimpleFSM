# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## SimpleFSM Library

SimpleFSM is an Arduino/ESP library for finite state machines, targeting embedded systems. It supports Arduino IDE and PlatformIO with standard Arduino C++ (C++11/C++14) — no STL.

## Development Commands

```bash
# Run unit tests (no hardware required — fastest)
pio test -e epoxy-esp8266        # ~3-5s
pio test -e epoxy-esp32          # ~3-5s

# Run with verbose output
pio test -e epoxy-esp8266 -v

# Hardware tests
pio test -e Wemos_test           # ESP8266
pio test -e M5Stack_ESP32_test   # ESP32
pio test -e Nano_test            # Arduino Nano

# Compile examples across all platforms
./compile_examples.sh            # from root
cd test && ./compile_examples.sh # from test directory

# Build for specific platforms (no tests)
pio run -e Wemos                 # ESP8266
pio run -e M5Stack_ESP32         # ESP32
pio run -e Nano                  # Arduino Nano
```

**Prerequisites**: `arduino-cli` in PATH, cores installed:

```bash
arduino-cli core install esp8266:esp8266 esp32:esp32 arduino:avr
```

## Architecture

### Source Files (`src/`)

- **`SimpleFSM.h/.cpp`** — Main FSM controller. Manages states/transitions, runs the tick loop, exposes the full public API.
- **`State.h/.cpp`** — Represents a single FSM state with `on_enter`, `on_state`, `on_exit` callbacks and an `is_final` flag.
- **`Transitions.h/.cpp`** — `AbstractTransition` base class; `Transition` (event-driven) and `TimedTransition` (time-based) subclasses. Both accept either `State*` pointers or `String` state names in constructors.
- **`FSMUtils.h/.cpp`** — Static helpers: `createManyToOneTransitions()` and `createManyToOneTimedTransitions()` for reducing boilerplate.
- **`FSMTestHelper.h/.cpp`** — Exposes `SimpleFSM::changeToState()` as a `friend class` for test use only.

### Tests (`test/SimpleFSMTest/SimpleFSMTest.ino`)

7 test suites using AUnit + EpoxyDuino. See `test/CLAUDE.md` for test-specific rules.

### Key Design Points

- **Global transitions**: Use `NULL` as `from` in `Transition(NULL, &target, event)`, or the cleaner `fsm.addGlobalTransition(&target, event)` / `fsm.addGlobalTimedTransition(&target, interval)`.
- **Safety limits**: `MAX_STATES=50`, `MAX_TRANSITIONS=100`, `MAX_TIMED_TRANSITIONS=50`. All `add()` methods return `FSMError`.
- **DOT output**: `fsm.getDotDefinition()` generates GraphViz markup for visualization.
- **Internal storage**: Dynamic arrays grow via `new`/`delete`. States stored as `State**`, transitions as `Transition*` / `TimedTransition*`.

## C++ Constraints

This is embedded Arduino C++ — no STL allowed:

| ❌ Don't use | ✅ Use instead |
| --- | --- |
| `std::vector`, `std::string`, `std::function` | C-style arrays, Arduino `String`, function pointers |
| `std::shared_ptr`, `std::unique_ptr` | Raw pointers with `new`/`delete` |
| Lambda functions | Named function pointers: `void (*cb)()` |
| `auto` with complex types | Explicit types |
| `nullptr` (in test code) | `NULL` — required for Arduino compatibility |
| `try`/`catch` | Return `FSMError` codes |

## API Quick Reference

```cpp
// Constructors
SimpleFSM fsm;
SimpleFSM fsm(&initial_state);

// Adding components (all return FSMError)
fsm.add(transitions, count);         // Transition[]
fsm.add(timedTransitions, count);    // TimedTransition[]
fsm.add(state_ptrs, count);          // State*[]  — NOT State[]

// Global transitions (v2.0+ helpers)
fsm.addGlobalTransition(&target, event_id);
fsm.addGlobalTransition(&target, event_id, callback);
fsm.addGlobalTimedTransition(&target, interval_ms);
fsm.addGlobalTimedTransition(&target, interval_ms, callback);

// Control
fsm.setInitialState(&state);
fsm.trigger(event_id);              // event IDs must start from 1, not 0
fsm.run(interval_ms);               // call in loop(); default 1000ms
fsm.reset();

// Status
fsm.getState();          // returns initial_state before first run/trigger or after reset()
fsm.getPreviousState();
fsm.isInState(&state);
fsm.isFinished();
fsm.lastTransitioned();

// Error handling
fsm.hasError();
fsm.getLastError();                  // returns FSMError enum
fsm.getErrorString(error);

// Handlers
fsm.setTransitionHandler(callback);
fsm.setFinishedHandler(callback);
```

## Critical Testing Rules

See `test/CLAUDE.md` for the full guide. Key rules:

1. **Test isolation**: Always create local `SimpleFSM` and `State` instances — never global. Call `resetCounters()` as the first line of every test.

2. **Use `NULL`, not `nullptr`**: Arduino environments may not support `nullptr`.

3. **Timed transitions**: Use `fsm.run(10)` (small interval) — the 1000ms default will miss short timeouts in tests.

4. **State pointer arrays**: `fsm.add()` requires `State*[]`, not `State[]`:

   ```cpp
   State s1("name", callback);
   State* ptrs[] = { &s1 };
   fsm.add(ptrs, 1);
   ```
