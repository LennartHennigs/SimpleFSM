# SimpleFSM

Arduino/ESP library to simplify setting up and running a state machine.

* Author: Lennart Hennigs (<https://www.lennarthennigs.de>)
* Copyright (C) 2022 Lennart Hennigs.
* Released under the MIT license.

## Description

* This library allows you to quickly setup a State Machine.
* Read here [what a state machine is](https://majenko.co.uk/blog/our-blog-1/the-finite-state-machine-26) and [why a state mache is neat for hardware projects](https://barrgroup.com/embedded-systems/how-to/state-machines-event-driven-systems?utm_source=pocket_mylist)

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
* Functions for tracking the behaviour and progress of the state machine
* Creation of a `Graphviz` source file of your state machine definition
* TBD: Storage of the current state on files with `LittleFS` or SD card storage (TBD)
  
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
  
  ```c++
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
* You can add both types to a state machine, see [MixedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/MixedTransitions/MixedTransitions.ino) for an example
* See [Transitions.h](https://github.com/LennartHennigs/SimpleFSM/blob/master/src/Transitions.h) for the class definitions of `Transition` and `TimedTransition`.
* Note: Both classes are based of an abstract class which is not to be used in your code.

### Regular Transitions

* Regular transitions must have a a set of states and a trigger (ID):

  ```c++
  Transition transitions[] = {
    Transition(&s[0], &s[1], light_switch_flipped),
    Transition(&s[1], &s[0], light_switch_flipped)
  };
  ```

* Define the triggeres for your state machine in an enum (and set the first enum value to 1, to be safe):

  ```c++
  enum triggers {
    light_switch_flipped = 1  
  };
  ```

* To call a trigger use the `trigger()`function:

  ```c++
  fsm.trigger(light_switch_flipped);
  ```

* See [SimpleTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/SimpleTransitions/SimpleTransitions.ino) and [SimpleTransitionWithButtons.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/SimpleTransitionWithButton/SimpleTransitionWithButton.ino) for more details

### Timed Transitions

* Timed transitions are automatically executed after a certain amount of time has passed
* The interval is defined in milliseconds:

  ```c++
  TimedTransition timedTransitions[] = {
    TimedTransition(&s[0], &s[1], 6000),
    TimedTransition(&s[1], &s[0], 4000),
    TimedTransition(&s[2], &s[1], 2000)
  };
  ```

* See [TimedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/TimedTransitions/TimedTransitions.ino) for more details

### Guard Conditions

* Guard conditions are evaluated before a transition takes places
* Only if the guard condition is true the transition will be executed
* Both regular and timed transitions can have guard conditions
* You define guard conditions as functions:

  ```c++
  TimedTransition timedTransitions[] = {
    TimedTransition(&s[0], &s[1], 1000, NULL, "", zero_yet),
    TimedTransition(&s[0], &s[0], 1000, NULL, "", not_zero_yet)
  };  

  bool not_zero_yet() {
    return countdown != 0;
  }

  bool zero_yet() {
    return countdown == 0;
  }
  ```

* See [Guards.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/Guards/Guards.ino) for a complete example

### In-State Interval

* States can have up to three events that are triggered, while they are in the state, and when they leave the state:

  ```c++
  State(String name, CallbackFunction on_enter, CallbackFunction on_state = NULL, CallbackFunction on_exit = NULL, bool is_final = false);
  ```

* To define how frequent the `on_state` event is called you can pass an interval (in ms) to the `run()` function:

  ```c++
  void run(int interval = 1000, CallbackFunction tick_cb = NULL);
  ```

* After this interval has passed the `on_state` function of the active state will be called (if it defined)
* Also the `tick_cb`callback function will be called (if defined) in the `run()` call

### Helper functions

* SimpleFSM provides a few functions to check on the state of the machine:

  ```c++
    State* getState() const;
    State* getPreviousState() const;
    bool isInState(State* state) const;
    int lastTransitionedAt() const;
    bool isFinished() const;

  ```

### GraphViz Generation

* Use the function `getDotDefinition()` to get your state machine definition in the GraphViz [dot format](https://www.graphviz.org/doc/info/lang.html)
* Here the output for the [MixedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/MixedTransitions/MixedTransitions.ino) example:
  
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
  ![MixedTransitions.ino example](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/MixedTransitions/MixedTransitions.svg?raw=true)

* If the machine is running, the current state will be highlighted
* Currently guard functions and end states are not shown in the graph
* See [MixedTransitionsBrowser.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/MixedTransitionsBrowser/MixedTransitionsBrowser.ino) to learn how to run a webserver to show the Graphviz diagram of your state machine

## Class Definitions

* [State.h](https://github.com/LennartHennigs/SimpleFSM/blob/master/src/State.h)
* [Transitions.h](https://github.com/LennartHennigs/SimpleFSM/blob/master/src/Transitions.h) for the class definition of both transitions
* [SimpleFSM](https://github.com/LennartHennigs/SimpleFSM/blob/master/src/SimpleFSM.h)

## Examples

* [SimpleTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/SimpleTransitions/SimpleTransitions.ino) - only regular transitions and showcasing the different events
* [SimpleTransitionWithButtons.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/SimpleTransitionWithButton/SimpleTransitionWithButton.ino) - event is now triggered via a hardware button
* [TimedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/TimedTransitions/TimedTransitions.ino) - showcasing timed transitions
* [MixedTransitions.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/MixedTransitions/MixedTransitions.ino) - regular and timed transitions
* [MixedTransitionsBrowser.ino](https://github.com/LennartHennigs/SimpleFSM/blob/master/examples/MixedTransitionsBrowser/MixedTransitionsBrowser.ino) - creates a webserver to show the Graphviz diagram of the state machine
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
