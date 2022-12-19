/////////////////////////////////////////////////////////////////
/*
    This example showcases how guards work.
    It emulates a time bomb count down.

    As long as the countdown is not zero the FSM stays in the counting state.
    When it reaches zero it goes to the exploding state.
*/
/////////////////////////////////////////////////////////////////

#include "SimpleFSM.h"

/////////////////////////////////////////////////////////////////

int countdown = 6;

SimpleFSM fsm;

/////////////////////////////////////////////////////////////////

void counting() {
  Serial.println(countdown--);
}

void boom() {
  Serial.println("BOOM");
}

bool not_zero_yet() {
  return !zero_yet();
}

bool zero_yet() {
  return countdown == 0;
}

void finished() {
  Serial.println("THE END!");
}

void tick() {
  Serial.println("TICK");
}


/////////////////////////////////////////////////////////////////

State s[] = {
  State("counting",   counting),
  State("exploding",  boom)
};

TimedTransition timedTransitions[] = {
  TimedTransition(&s[0], &s[0], 1000, NULL, "", not_zero_yet)
  TimedTransition(&s[0], &s[1], 1000, NULL, "", zero_yet),
};

int num_timed = sizeof(timedTransitions) / sizeof(TimedTransition);

/////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(300);
  }
  Serial.println();
  Serial.println("SimpleFSM - Using Guard Condition (Time Bomb)\n");
    
  fsm.add(timedTransitions, num_timed);
  // initial state  
  fsm.setInitialState(&s[0]);
    // final state    
  s[1].setAsFinal(true);
  fsm.setFinishedHandler(finished); 
}

/////////////////////////////////////////////////////////////////

void loop() {
  fsm.run(1000, tick);
}

/////////////////////////////////////////////////////////////////