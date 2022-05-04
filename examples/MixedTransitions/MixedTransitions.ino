/////////////////////////////////////////////////////////////////

#include "SimpleFSM.h"
#include "Button2.h"    // https://github.com/LennartHennigs/Button2

/////////////////////////////////////////////////////////////////

#define BUTTON_PIN  D4

/////////////////////////////////////////////////////////////////

SimpleFSM fsm;
Button2 btn;

/////////////////////////////////////////////////////////////////

void on_red() {
  Serial.println("State: RED");
}
 
void on_green() {
  Serial.println("State: GREEN");
}

void on_button_press() {
  Serial.println("BUTTON!");
}

/////////////////////////////////////////////////////////////////

State s[] = {
  State("red",        on_red),
  State("green",      on_green),
  State("BTN",        on_button_press)
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
  TimedTransition(&s[2], &s[1], 2000)
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