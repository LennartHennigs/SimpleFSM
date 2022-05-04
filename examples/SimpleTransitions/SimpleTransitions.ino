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
  Serial.println("Leaving State: ON ");
}
 
void exit_light_off() {
  Serial.println("Leaving State: OFF");
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

State s[] = {
  State("on",   light_on,   ongoing,  exit_light_on),
  State("off",  light_off,  NULL,     exit_light_off)
};

enum triggers {
  light_switch_flipped = 1  
};

Transition transitions[] = {
  Transition(&s[0], &s[1], light_switch_flipped, on_to_off),
  Transition(&s[1], &s[0], light_switch_flipped, off_to_on)
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
  fsm.setInitialState(&s[1]);
}

/////////////////////////////////////////////////////////////////

void loop() {
  fsm.run();
  // flip the switch every 4 seconds
  // better than using delay() as this won't block the loop()
  if (fsm.getTimeSinceTransition() > 4000) {
    fsm.trigger(light_switch_flipped);
  }
}

/////////////////////////////////////////////////////////////////