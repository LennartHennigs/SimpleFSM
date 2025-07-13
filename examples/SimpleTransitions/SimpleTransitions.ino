/////////////////////////////////////////////////////////////////
/*
    This emulates an blinken light.
    Every four seconds will the light turn on or off.
    While the light is "on", a "." will be shown.
*/

/////////////////////////////////////////////////////////////////

#include "SimpleFSM.h"

/////////////////////////////////////////////////////////////////

SimpleFSM fsm;

/////////////////////////////////////////////////////////////////

void light_on() {
  Serial.println("Entering State: ON");
}
 
void light_off() {
  Serial.println("Entering State: OFF");
}

void exit_light_on() {
  Serial.println("\nLeaving State: ON ");
}
 
void exit_light_off() {
  Serial.println("\nLeaving State: OFF");
}

void on_to_off() {
  Serial.println("ON -> OFF");
}

void off_to_on() {
  Serial.println("OFF -> ON");  
}

void ongoing() {
  Serial.print(".");  
}

/////////////////////////////////////////////////////////////////


State s_on("on", light_on, ongoing, exit_light_on);
State s_off("off", light_off, ongoing, exit_light_off);
State* states[] = { &s_on, &s_off };

enum triggers {
  light_switch_flipped = 1  
};

Transition transitions[] = {
  Transition(states[0], states[1], light_switch_flipped, on_to_off),
  Transition(states[1], states[0], light_switch_flipped, off_to_on)
};

int num_transitions = sizeof(transitions) / sizeof(Transition);

/////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(300);
  }
  Serial.println();
  Serial.println();
  Serial.println("SimpleFSM - Simple Transitions (Light Switch)\n");
    
  fsm.add(transitions, num_transitions);
  fsm.setInitialState(states[1]);
}

/////////////////////////////////////////////////////////////////

void loop() {
  fsm.run();
  // flip the switch every 4 seconds
  // better than using delay() as this won't block the loop()
  if (fsm.lastTransitioned() > 4000) {
    fsm.trigger(light_switch_flipped);
    Serial.println(fsm.getLastTransition()->getName());
  }
}

/////////////////////////////////////////////////////////////////