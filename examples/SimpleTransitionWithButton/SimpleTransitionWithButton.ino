/////////////////////////////////////////////////////////////////
/*
    This emulates a light switch.
    The button click will turn on/off the light.
    While the light is "on", a "." will be shown.
*/
/////////////////////////////////////////////////////////////////

#include "SimpleFSM.h"
#include "Button2.h"    // https://github.com/LennartHennigs/Button2

/////////////////////////////////////////////////////////////////

#define BUTTON_PIN  39

/////////////////////////////////////////////////////////////////

SimpleFSM fsm;
Button2 btn;

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

void button_handler(Button2 &btn) {
  if (fsm.getState() == &s[0]) Serial.println();
  
  Serial.println("BUTTON: I was flipped");
  fsm.trigger(light_switch_flipped);
}

/////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(300);
  }
  Serial.println();
  Serial.println();
  Serial.println("SimpleFSM - Simple Transitions (Light Switch w/ Button)\n");
    
  fsm.add(transitions, num_transitions);
  
  fsm.setInitialState(&s[1]);

  btn.begin(BUTTON_PIN);
  btn.setTapHandler(button_handler);  
}

/////////////////////////////////////////////////////////////////

void loop() {
    fsm.run(1000);
    btn.loop();
}

/////////////////////////////////////////////////////////////////