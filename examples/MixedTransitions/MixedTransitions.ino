/////////////////////////////////////////////////////////////////
/*
    This example shows how to use regular and timed transitions.
    The FSM emulates a traffic light.
    
    When the button is pressed the light will turn green after a second.
    Otherwise the light will transition from red to green every 6 seconds.
    And from green to red after 4.
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

void on_red() {
  Serial.println("\nState: RED");
}
 
void on_green() {
  Serial.println("\nState: GREEN");
}

void ongoing() {
  Serial.print(".");
}

void on_button_press() {
  Serial.println("\nBUTTON!");
}

/////////////////////////////////////////////////////////////////

State s[] = {
  State("red light", on_red, ongoing),
  State("green light", on_green, ongoing),
  State("button pressed", on_button_press, ongoing)
};

enum triggers {
  button_was_pressed = 1  
};

Transition transitions[] = {
  Transition(&s[0], &s[2], button_was_pressed)
};

TimedTransition timedTransitions[] = {
  TimedTransition(&s[0], &s[1], 6000),
  TimedTransition(&s[1], &s[0], 4000),
  TimedTransition(&s[2], &s[1], 1000)
};

int num_transitions = sizeof(transitions) / sizeof(Transition);
int num_timed = sizeof(timedTransitions) / sizeof(TimedTransition);

/////////////////////////////////////////////////////////////////

void button_handler(Button2 btn) {
  fsm.trigger(button_was_pressed);
}

/////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(300);
  }
  Serial.println();
  Serial.println();
  Serial.println("SimpleFSM - Mixed Transition (Traffic light with button for green)\n");
    
  fsm.add(timedTransitions, num_timed);
  fsm.add(transitions, num_transitions);
  
  fsm.setInitialState(&s[0]);

  btn.begin(BUTTON_PIN);
  btn.setTapHandler(button_handler); 
}

/////////////////////////////////////////////////////////////////

void loop() {
  fsm.run();
  btn.loop();
}

/////////////////////////////////////////////////////////////////