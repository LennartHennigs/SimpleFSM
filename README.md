# SimpleFSM

Arduino/ESP library to simplify setting up and running a state machinee.

* Author: Lennart Hennigs (<https://www.lennarthennigs.de>)
* Copyright (C) 2022 Lennart Hennigs.
* Released under the MIT license.

**Disclaimer**

* This library is not complete (yet) and a work in progress. 
* Some of the features mentioned below might not be done yet.
* This document is also not complete

## Description

* This library allows you to quickly setup a State Machine.
* Read here [what a state machine is]() and [why a state mache is neat for hardware projects]() TBD

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
* Definition of an in_state intervall
* Functions for tracking the behaviour and progress of the state machine (TBD)
* Storage of the current state on files with `LittleFS` or SD card storage (TBD)
* Creation of a `Graphviz` source file of your state machine definition (TBD)
  
To see the latest changes to the library please take a look at the [Changelog](https://github.com/LennartHennigs/SimpleFSM/blob/master/CHANGELOG.md).

## How To Use

* You first need to define a set of states for your state machine
* Then, define transitions (regular or timed) for these state
* Pass the transitions to your state machine
* Define an inital state
* ...and add the `run()` function in your loop
* See [SimpleTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/MixedTransitions/MixedTransitions.ino) for a basic example

### Defining States

* A finite state machine has a defined set of states
* A state must have a name...
* ...and can have callback functions for different events (when a state is entered, exited, or stays in a state)
* Most states will have the entry handler
* The easiest way to define states is creating using an array, e.g. as shown in [MixedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/MixedTransitions/MixedTransitions.ino):
  ```
  State s[] = {
    State("red",        on_red),
    State("green",      on_green),
    State("BTN",        on_button_press)
  };
  ```
* The inital state **must** of the state machine must be defined – either via the constructor or the `setup()` function or via the `setInitialState()` function
* None, one, or multiple states can be defined as an end state via `setAsFinal()`
* For the full `State` class definition see [State.h](https://github.com/LennartHennigs/SimpleFSM/blob/master/src/State.h)

### Transitions

* This library offers two types of Transitions, regular and timed ones
* All transitions must have a from and and a to state
* Transitions can have a callback function for when the transition is executed, a name, and a [guard condition](#guard-conditions)
* See [Transitions.h](https://github.com/LennartHennigs/SimpleFSM/blob/master/src/Transitions.h) for the class definition of both transitiond. (Note: Both classes are based of an abstract class which is not to be used in your code.)
* You can add both types to a state machine, see [MixedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/MixedTransitions/MixedTransitions.ino) for an example

### Regular Transitions
* Regular transitions need to be manually triggered, e.g. through a button press:
  ```
  Transition transitions[] = {
    Transition(&s[0], &s[2], button_was_pressed)
  };
  ```
* Regular transitions must have a trigger (ID), above this is `button_was_pressed`
* It's easy to define triggeres in an enum:
  ```
  enum triggers {
    button_was_pressed = 1  
  };
  ```

* See [SimpleTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/SimpleTransitions/SimpleTransitions.ino) and [SimpleTransitionWithButtons.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/SimpleTransitionWithButton/SimpleTransitionWithButton.ino)

### Timed Transitions 
 
* Timed transitions are automatically executed after a certain amount of time
* The interval is defined in milliseconds
  ```
  TimedTransition timedTransitions[] = {
    TimedTransition(&s[0], &s[1], 6000),
    TimedTransition(&s[1], &s[0], 4000),
    TimedTransition(&s[2], &s[1], 2000)
  };
  ```
* See [TimedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/TimedTransitions/TimedTransitions.ino)

### Guard Conditions

* ...
* See [Guards.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/Guards/Guards.ino)

### In-State Interval

* ...

### Helper functions

* ...

## Class Definition

* [State.h](https://github.com/LennartHennigs/SimpleFSM/blob/master/src/State.h)
* [Transitions.h](https://github.com/LennartHennigs/SimpleFSM/blob/master/src/Transitions.h) for the class definition of both transitions
* [SimpleFSM](https://github.com/LennartHennigs/SimpleFSM/blob/master/src/SimpleFSM.h)

## Examples

* [SimpleTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/SimpleTransitions/SimpleTransitions.ino) - only regular transitions and showcasing the different events
* [SimpleTransitionWithButtons.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/SimpleTransitionWithButton/SimpleTransitionWithButton.ino) - event is now triggered via a hardware button
* [TimedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/TimedTransitions/TimedTransitions.ino) - showcasing timed transitions
* [MixedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/MixedTransitions/MixedTransitions.ino) - regular and timed transitions
* [Guards.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/Guards/Guards.ino) - showing how to define guard functions

## Notes

* This libary is heavily inspiried by the [Arduino-fsm](https://github.com/jonblack/arduino-fsm) library created by [Jon Black](https://github.com/jonblack). I initally used some of his as a base. Without Jon's work this library would not exist.
* To see the latest changes to the library please take a look at the [Changelog](https://github.com/LennartHennigs/SimpleFSM/blob/master/CHANGELOG.md).
* And if you find this library helpful, please consider giving it a star at [GitHub](https://github.com/LennartHennigs/SimpleFSM). Thanks!

## How To Install

Open the Arduino IDE choose "Sketch > Include Library" and search for "SimpleFSM".
Or download the ZIP archive (<https://github.com/lennarthennigs/SimpleFSM/zipball/master>), and choose "Sketch > Include Library > Add .ZIP Library..." and select the downloaded file.

## License

MIT License

Copyright (c) 2022 Lennart Hennigs

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
