/////////////////////////////////////////////////////////////////

#include "SimpleFSM.h"
#include "Button2.h"

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
  Serial.println("SimpleFSM - Mixed Transition (Traffic light with button for green)\n");
    
  fsm.add(timedTransitions, 3);
  fsm.add(transitions, 1);
  
  fsm.setInitialState(&s[0]);

  btn.begin(D4);
  btn.setTapHandler(button_handler);  
}

/////////////////////////////////////////////////////////////////

void loop() {
  fsm.run();
  btn.loop();
}

/////////////////////////////////////////////////////////////////