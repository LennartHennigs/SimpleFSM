# Changelog

## Unreleased

## 2.0.0 - 2026-02-26

- Breaking API changes
  - FSM now requires states to be passed as arrays of pointers (`State*[]`), not arrays of objects.
  - `add()` methods now return `FSMError` instead of `void` for better error handling.
  - `addUniqueState()` now returns `FSMError` instead of `void`.
- Added `FSMUtils.h` helper class – offers functions to generate many-to-one regular and timed transitions (see #26).
- Added global transitions feature
  - Transitions can now use `NULL` as the source state to work from any state.
  - NEW: Added convenient helper functions for global transitions:
    - `addGlobalTransition(state, event)` and `addGlobalTransition(state, event, callback)`
    - `addGlobalTimedTransition(state, interval)` and `addGlobalTimedTransition(state, interval, callback)`
  - Helper functions provide cleaner, more explicit API than using `NULL` in constructors.
  - Useful for emergency stops, error handling, and system-wide events.
  - Works with both regular and timed transitions.
  - Maintains full backward compatibility with existing code.
- Fixed bugs
  - Timed transitions now correctly reset their timer on state re-entry (see #25).
  - Fixed constructor bug where `SimpleFSM()` call had no effect in parameterized constructor.
  - Improved pointer safety and memory management for states and transitions.
- Added memory safety and bounds checking
  - Implemented configurable maximum limits for states, transitions, and timed transitions.
  - Added comprehensive input validation for all `add()` methods.
  - Replaced dangerous `abort()` calls with proper error codes.
  - Added null pointer checking throughout the API.
  - Memory allocation failures are now handled gracefully.
- Added error handling system
  - New `FSMError` enum for standardized error reporting.
  - `getLastError()`, `hasError()`, and `getErrorString()` methods.
  - Error state is automatically reset on `reset()` calls.
- Improved code maintainability
  - Replaced magic numbers with meaningful named constants.
  - Added configuration constants for timing, limits, and DOT formatting.
  - Enhanced code readability with descriptive constant names.
- Updated examples
  - Added `ManyToOneTransitionExample.ino` to demonstrate usage new helpers (see #26).
  - Updated `SimpleTransitions.ino` to demonstrate error handling.
  - Added constants to some examples
  - They now use array of pointers for states
- Added unit tests
  - Added AUnit tests for FSM, transitions, guards, and callbacks.
  - Added comprehensive memory safety and error handling tests.
  - Exposed protected FSM internals for testing via friend class (`FSMTestHelper`).
- Added comprehensive testing infrastructure
  - New compilation test script (`test/compile_examples.sh`) for automated platform testing.
  - Tests compilation across ESP8266 (Wemos D1 Mini), ESP32 (M5Stack Core2), and Arduino Nano.
  - Automatic platform-specific example exclusion handling.
  - Colored output with detailed error reporting for failed compilations.
- Improved documentation and README to reflect new pointer-based API and usage patterns.
  - Added comprehensive migration guide from v1.x to v2.0 in README.md with step-by-step instructions.
  - Added CLAUDE.md - AI assistant reference guide with Arduino/PlatformIO constraints and best practices.
  - Enhanced README with migration checklist, before/after code examples, and rationale for changes.
- Added functions
  - `getStateByName()`
  - `getLastTransition()`
  - `getStateCount()`
  - `isSetupOK()`
  - `checkAndInitializeTransitions()`
  - `add(State* states[], int size)`
- Renamed protected functions
  - `_dot_initial_state()` to `getDOTInitialState()`
  - `_dot_active_node()` to `getDOTActiveNode()`
  - `_dot_header()` to `getDOTHeader()`
  - removed trailing `_`for all protected functions
- Updated function
  - `getDotDefinition(bool showActive /* = TRUE */ )`

## 1.3.1 - 2024-10-24

- fixed Wrong type of memcopy argument #16

## 1.3.0 - 2023-10-14

- Refactored `run()`
- Updated `add()` – now multiple transition arrays can be added
- Added `getTransitionCount()` and `getTimedTransitionCount()` functions as suggested in [PR #7](https://github.com/LennartHennigs/SimpleFSM/pull/7)
- Fixed `TimedTransitions` constructor as mentioned in [#9](https://github.com/LennartHennigs/SimpleFSM/issues/9)
- Fixed error in `State` constructor as mentioned in [#12](https://github.com/LennartHennigs/SimpleFSM/issues/12)
- Fixed memory handling in `SimpleFSM` destructor as mentioned in [#8](https://github.com/LennartHennigs/SimpleFSM/issues/8)
- Updated the button handlers in `MixedTransitions.ino`and `SimpleTransitionWithButton.ino` as mentioned in [#4](https://github.com/LennartHennigs/SimpleFSM/issues/4)

## 1.2.0 - 2022-12-19

- Refactored code
- Changed `bool add()` to `void add()`
- Fixed bug, that the "ongoing state" was also called when a timed transition happens
- Remove low-level memory allocation commands to remove compiler warnings mentioned in [#2](https://github.com/LennartHennigs/SimpleFSM/issues/2)
- Updated examples (added explanation and some additional callbacks to show the state of the FSM)

## 1.1.0 - 2022-05-30

- Added fix for ESP32 crashes as reported by [Erik](https://github.com/snowrodeo) in [#1](https://github.com/LennartHennigs/SimpleFSM/issues/1)

## 1.0.1 - 2022-05-07

- Removed compile warnings for `SimpleFSM.cpp`
- Updated `.gitignore`

## 1.0.0 - 2022-05-07

- Initial release

## Note

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
