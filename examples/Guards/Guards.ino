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

// Constants for timing and initialization
#define SERIAL_SPEED 9600
#define COUNTDOWN_INTERVAL_MS 1000
#define INITIAL_COUNTDOWN_VALUE 6

/////////////////////////////////////////////////////////////////

int countdown = INITIAL_COUNTDOWN_VALUE;

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


State s_counting("counting", counting);
State s_exploding("exploding", boom);
State* states[] = { &s_counting, &s_exploding };

TimedTransition timedTransitions[] = {
  TimedTransition(states[0], states[0], COUNTDOWN_INTERVAL_MS, NULL, "", not_zero_yet),
  TimedTransition(states[0], states[1], COUNTDOWN_INTERVAL_MS, NULL, "", zero_yet)
};
int num_timed = sizeof(timedTransitions) / sizeof(TimedTransition);

/////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(SERIAL_SPEED);
  while (!Serial) {
    delay(300);
  }
  Serial.println();
  Serial.println("SimpleFSM - Using Guard Condition (Time Bomb)\n");
    
  fsm.add(timedTransitions, num_timed);
  // initial state  
  fsm.setInitialState(states[0]);
  // final state    
  states[1]->setAsFinal(true);
  fsm.setFinishedHandler(finished); 
}

/////////////////////////////////////////////////////////////////

void loop() {
  fsm.run(COUNTDOWN_INTERVAL_MS, tick);
}

/////////////////////////////////////////////////////////////////