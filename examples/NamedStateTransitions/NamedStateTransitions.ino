/////////////////////////////////////////////////////////////////
/*
  NamedStateTransitions.ino

  Shows how to define transitions using state names (strings)
  instead of State* pointers. Transitions and states can be
  declared in any order — names are resolved automatically
  on the first run() or trigger() call.

  Scenario: a simple coffee machine with three states.
    Standby  --[BREW_BUTTON]--> Brewing
    Brewing  --[10s timeout]--> Ready
    Ready    --[COFFEE_TAKEN]--> Standby
*/
/////////////////////////////////////////////////////////////////

#include "SimpleFSM.h"

/////////////////////////////////////////////////////////////////

#define SERIAL_SPEED    9600
#define BREW_BUTTON     1
#define COFFEE_TAKEN    2
#define BREW_TIME_MS    10000
#define SIM_DELAY_MS    2000   // how long to wait before simulating input

/////////////////////////////////////////////////////////////////
// Transitions declared by name before the State objects exist.
// This demonstrates order independence: you can define transitions
// in a separate file or at the top of a large sketch without
// needing forward declarations or State* pointers at that point.

Transition transitions[] = {
  Transition("Standby", "Brewing", BREW_BUTTON),
  Transition("Ready",   "Standby", COFFEE_TAKEN),
};
int num_transitions = sizeof(transitions) / sizeof(Transition);

TimedTransition timedTransitions[] = {
  TimedTransition("Brewing", "Ready", BREW_TIME_MS),
};
int num_timed = sizeof(timedTransitions) / sizeof(TimedTransition);

/////////////////////////////////////////////////////////////////

void onEnterStandby() { Serial.println("State: Standby — press button to brew"); }
void onEnterBrewing() { Serial.println("State: Brewing..."); }
void onEnterReady()   { Serial.println("State: Coffee ready!"); }

State standby("Standby", onEnterStandby);
State brewing("Brewing", onEnterBrewing);
State ready  ("Ready",   onEnterReady);

State* allStates[] = { &standby, &brewing, &ready };
int num_states = sizeof(allStates) / sizeof(State*);

SimpleFSM fsm;

/////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(SERIAL_SPEED);
  while (!Serial) delay(300);
  Serial.println("\nSimpleFSM - Named State Transitions\n");

  fsm.add(allStates, num_states);
  fsm.add(transitions, num_transitions);
  fsm.add(timedTransitions, num_timed);
  fsm.setInitialState(&standby);
  // String names are resolved on the first run() or trigger() call.
}

/////////////////////////////////////////////////////////////////

void loop() {
  fsm.run();

  // Simulate the brew button 2 seconds after reaching Standby.
  if (fsm.isInState(&standby) && fsm.lastTransitioned() > SIM_DELAY_MS) {
    Serial.println("(Simulating: brew button pressed)");
    fsm.trigger(BREW_BUTTON);
  }

  // Collect the coffee 2 seconds after it is ready.
  if (fsm.isInState(&ready) && fsm.lastTransitioned() > SIM_DELAY_MS) {
    Serial.println("(Simulating: coffee collected)");
    fsm.trigger(COFFEE_TAKEN);
  }
}

/////////////////////////////////////////////////////////////////
