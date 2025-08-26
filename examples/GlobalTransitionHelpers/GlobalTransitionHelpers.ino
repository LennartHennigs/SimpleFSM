/////////////////////////////////////////////////////////////////
/*
  GlobalTransitionHelpers.ino
  
  Demonstrates the complete set of global transition helper functions
  in SimpleFSM. Shows event-based and timed global transitions that
  work from ANY state.
  
*/
/////////////////////////////////////////////////////////////////

#include "SimpleFSM.h"

/////////////////////////////////////////////////////////////////
void stateEnter() {
  // Placeholder callback function
}

State normal;
State emergency;  
State maintenance;

SimpleFSM fsm;

/////////////////////////////////////////////////////////////////
// Events
#define EMERGENCY_STOP    1
#define MAINTENANCE_MODE  2
#define SYSTEM_RESET      3

void setup() {
  Serial.begin(9600);
  Serial.println("SimpleFSM Global Transition Helper Demo");
  
  normal.setup("NORMAL", stateEnter);
  emergency.setup("EMERGENCY", stateEnter);
  maintenance.setup("MAINTENANCE", stateEnter);
  
  fsm.setInitialState(&normal);
  
  State* allStates[] = {&normal, &emergency, &maintenance};
  fsm.add(allStates, 3);
  
  // Global transitions using helper functions
  fsm.addGlobalTransition(&emergency, EMERGENCY_STOP);
  fsm.addGlobalTransition(&maintenance, MAINTENANCE_MODE);
  fsm.addGlobalTimedTransition(&normal, 30000);  // Auto-reset every 30s from any state
  
  // Regular state-specific transitions
  Transition normalTransitions[] = {
    Transition(&emergency, &normal, SYSTEM_RESET),
    Transition(&maintenance, &normal, SYSTEM_RESET)
  };
  fsm.add(normalTransitions, 2);
  
  Serial.println("Global transitions configured:");
  Serial.println("- Emergency stop from ANY state");
  Serial.println("- Maintenance mode from ANY state");
  Serial.println("- Auto-reset to normal every 30s from ANY state");
  Serial.println();
}

void loop() {
  static unsigned long lastDemo = 0;
  static int demoStep = 0;
  
  if (millis() - lastDemo > 5000) {
    switch (demoStep) {
      case 0:
        Serial.println("Triggering EMERGENCY_STOP from " + fsm.getState()->getName());
        fsm.trigger(EMERGENCY_STOP);
        break;
      case 1:
        Serial.println("Triggering MAINTENANCE_MODE from " + fsm.getState()->getName());
        fsm.trigger(MAINTENANCE_MODE);
        break;
      case 2:
        Serial.println("Triggering SYSTEM_RESET from " + fsm.getState()->getName());
        fsm.trigger(SYSTEM_RESET);
        break;
    }
    
    demoStep = (demoStep + 1) % 3;
    lastDemo = millis();
  }
  
  fsm.run();
  
  static String lastStateName = "";
  String currentStateName = fsm.getState()->getName();
  if (currentStateName != lastStateName) {
    Serial.println("Current State: " + currentStateName);
    lastStateName = currentStateName;
  }
}
