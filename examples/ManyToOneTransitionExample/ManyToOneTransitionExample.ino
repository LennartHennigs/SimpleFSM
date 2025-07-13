/////////////////////////////////////////////////////////////////
// ManyToOneTransitionExample.ino - Demonstrates utility helpers for many-to-one transitions
/////////////////////////////////////////////////////////////////
#include "SimpleFSM.h"
#include "FSMUtils.h"

/////////////////////////////////////////////////////////////////
#define SERIAL_SPEED 115200
#define HOME_EVENT 1
#define TIME_TO_HOME 5000

/////////////////////////////////////////////////////////////////
SimpleFSM fsm;

/////////////////////////////////////////////////////////////////
void on_screen1() { Serial.println("Screen 1"); }
void on_screen2() { Serial.println("Screen 2"); }
void on_screen3() { Serial.println("Screen 3"); }
void on_home()    { Serial.println("Home Screen"); }

/////////////////////////////////////////////////////////////////
State s1("Screen1", on_screen1);
State s2("Screen2", on_screen2);
State s3("Screen3", on_screen3);
State home("Home", on_home);
State* screens[] = { &s1, &s2, &s3 };

/////////////////////////////////////////////////////////////////

Transition homeTransitions[3];
TimedTransition timedHomeTransitions[3];


/////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(SERIAL_SPEED);
  while (!Serial) { delay(300); }
  Serial.println();
  Serial.println("SimpleFSM - Many-to-One Transition Example\n");

  FSMUtils::createManyToOneTransitions(screens, 3, &home, HOME_EVENT, homeTransitions);
  FSMUtils::createManyToOneTimedTransitions(screens, 3, &home, TIME_TO_HOME, timedHomeTransitions);

  fsm.add(homeTransitions, 3);
  fsm.add(timedHomeTransitions, 3);
  fsm.setInitialState(&s1);
}

/////////////////////////////////////////////////////////////////
void loop() {
  fsm.run();
  // Simulate HOME_EVENT trigger every 10 seconds
  static unsigned long lastTrigger = 0;
  if (millis() - lastTrigger > 10000) {
    fsm.trigger(HOME_EVENT);
    lastTrigger = millis();
  }
}
/////////////////////////////////////////////////////////////////
