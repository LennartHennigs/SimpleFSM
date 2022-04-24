/////////////////////////////////////////////////////////////////

#include "SimpleFSM.h"

/////////////////////////////////////////////////////////////////

int countdown = 6;

/////////////////////////////////////////////////////////////////

void counting() {
  Serial.println(--countdown);
}

void boom() {
  Serial.println("BOOM");
}

bool not_zero_yet() {
  return countdown != 0;
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

SimpleFSM fsm;

State s[] = {
  State("counting",   counting),
  State("exploding",  boom)
};

TimedTransition timedTransitions[] = {
  TimedTransition(&s[0], &s[1], 1000, NULL, "", zero_yet),
  TimedTransition(&s[0], &s[0], 1000, NULL, "", not_zero_yet)
};

/////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(300);
  }
  Serial.println();
  Serial.println("SimpleFSM - Using Guard Condition (Time Bomb)\n");
    
  fsm.add(timedTransitions, 2);
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