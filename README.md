SimpleFSM
=========

Arduino/ESP library to simplify setting up and running a state machinee.

* Author: Lennart Hennigs (<https://www.lennarthennigs.de>)
* Copyright (C) 2022 Lennart Hennigs.
* Released under the MIT license.

Description
-----------

* This library allows you to quickly setup a State Machine.

* It is heavily inspiried by the [Arduino-fsm](https://github.com/jonblack/arduino-fsm) library created by [Jon Black](https://github.com/jonblack). I also used some of its code as a base for this library. Without his work this library would not exist.

__Features__

* Event triggered transitions and (automatic) timed transitions
* Possibility to define guard conditions for your transitions
* Definition of and initial state and multiple final states
* Callback functions for...
  * State entry, staying, exit
  * Transition entry
  * Final state reached
  * run interval of the State Machine
* Creation of a `Graphviz` source file of your state machine definition
  
To see the latest changes to the library please take a look at the [Changelog](https://github.com/LennartHennigs/SimpleFSM/blob/master/CHANGELOG.md).

How To Use
==========

* You first need to define a set of states for your state machine
* Then, define regular or (automatic) timed transitions
* And finally pass the transitions to you state machine
* ...and let it run in your loop
* See [SimpleFSMBasic.ino]() for a basic examnple

__Defining States__
...

__Transisions__
...

__Guard Conditions__
...

__Helper functions__
...

Class Definition
----------------

Examples
========

Notes
=====

- To see the latest changes to the library please take a look at the [Changelog](https://github.com/LennartHennigs/SimpleFSM/blob/master/CHANGELOG.md).

* And if you find this library helpful, please consider giving it a star at [GitHub](https://github.com/LennartHennigs/SimpleFSM). Thanks!

------------
Open the Arduino IDE choose "Sketch > Include Library" and search for "SimpleFSM".
Or download the ZIP archive (<https://github.com/lennarthennigs/SimpleFSM/zipball/master>), and choose "Sketch > Include Library > Add .ZIP Library..." and select the downloaded file.

License
-------

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
