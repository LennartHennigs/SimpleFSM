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
#include "Button2.h"  // https://github.com/LennartHennigs/Button2

/////////////////////////////////////////////////////////////////

#define BUTTON_PIN 39
#define SERIAL_SPEED 115200

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

State s_red("red light", on_red, ongoing);
State s_green("green light", on_green, ongoing);
State s_button("button pressed", on_button_press, ongoing);
State* states[] = { &s_red, &s_green, &s_button };

enum triggers {
  button_was_pressed = 1
};

Transition transitions[] = {
  Transition(states[0], states[2], button_was_pressed)
};

TimedTransition timedTransitions[] = {
  TimedTransition(states[0], states[1], 6000),
  TimedTransition(states[1], states[0], 4000),
  TimedTransition(states[2], states[1], 1000)
};

int num_transitions = sizeof(transitions) / sizeof(Transition);
int num_timed = sizeof(timedTransitions) / sizeof(TimedTransition);

/////////////////////////////////////////////////////////////////

void button_handler(Button2 &btn) {
  fsm.trigger(button_was_pressed);
}

/////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(SERIAL_SPEED);
  while (!Serial) {
    delay(300);
  }
  Serial.println();
  Serial.println();
  Serial.println("SimpleFSM - Mixed Transition (Traffic light with button for green)\n");

  fsm.add(timedTransitions, num_timed);
  fsm.add(transitions, num_transitions);

  fsm.setInitialState(states[0]);

  btn.begin(BUTTON_PIN);
  btn.setTapHandler(button_handler);
}

/////////////////////////////////////////////////////////////////

void loop() {
  fsm.run();
  btn.loop();
}

/////////////////////////////////////////////////////////////////