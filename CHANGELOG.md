# Changelog

## Unreleased

- Updated `add()` – now multiple transition arrays can be added
- Added `getTransitionCount()` and `getTimedTransitionCount()` functions as suggested in [PR #7](https://github.com/LennartHennigs/SimpleFSM/pull/7)
- Fixed `TimedTransitions` constructor as mentioned in [#9](https://github.com/LennartHennigs/SimpleFSM/issues/9)
- Fixed error in `State` constructor as mentioned in [#12](https://github.com/LennartHennigs/SimpleFSM/issues/12)
- Fixed memory handling in `SimpleFSM` destructor as mentioned in [#8](https://github.com/LennartHennigs/SimpleFSM/issues/8)
- Updated the button handlers in `MixedTransitions.ino`and `SimpleTransitionWithButton.ino` as mentioned in [#4](https://github.com/LennartHennigs/SimpleFSM/issues/4)

**Note:** Unreleased changes are checked in but not part of an official release (available through the Arduino IDE or PlatfomIO) yet. This allows you to test WiP features and give feedback to them.

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
