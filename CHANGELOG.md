# Changelog

**Note:** Unreleased changes are checked in but not part of an official release (available through the Arduino IDE or PlatfomIO) yet. This allows you to test WiP features and give feedback to them.

## Unreleased
- Added `FSMTransitionUtils.h` utility header with helper functions for generating many-to-one regular and timed transitions (see #26).
- Added `ManyToOneTransitionExample.ino` to demonstrate usage of these helpers (see #26).
- Breaking API change: FSM now requires states to be passed as arrays of pointers (`State*[]`), not arrays of objects. All examples updated accordingly.
- Fixed bug: Timed transitions now correctly reset their timer on state re-entry (#25).
- Improved pointer safety and memory management for states and transitions.
- Added comprehensive AUnit test suite for FSM, transitions, guards, and callbacks.
- Exposed protected FSM internals for testing via friend class (`FSMTestHelper`).
- Updated all example sketches to use pointer arrays for state management.
- Improved documentation and README to reflect new pointer-based API and usage patterns.

- can now add state names to FSM via `add(State* states[], int size)` (pointer array API)
  - checks whether names are unique
  - determines if in end state
- added `getStateByName()`
- added `getLastTransition()`
- added `getStateCount()`
- renamed protected functions
  - removed trailing `_`for `_initFSM()`, `_transitionTo()`, `_isDuplicate()`, `_addDotTransition()`, `_isTimeForRun()`, `_handleTimedEvents()`, `_changeToState()`
  - renamed `_dot_initial_state()` to `getDOTInitialState()`
  - renamed `_dot_active_node()` to `getDOTActiveNode()`
  - renamed `_dot_header()` to `getDOTHeader()`
- updated `getDotDefinition(bool showActive /* = TRUE */ )`
- added protected functions
  - `isSetupOK()`
  - `checkAndInitializeTransitions()`

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
