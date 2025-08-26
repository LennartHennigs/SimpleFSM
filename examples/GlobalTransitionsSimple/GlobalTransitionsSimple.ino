/////////////////////////////////////////////////////////////////
/*
  GlobalTransitionsSimple.ino
  
  This simplified example demonstrates the addGlobalTransition() helper 
  functions in SimpleFSM. Global transitions work from ANY state and are 
  perfect for emergency stops and system-wide events.
  
*/
/////////////////////////////////////////////////////////////////

#include "SimpleFSM.h"

/////////////////////////////////////////////////////////////////
// Event definitions
enum Events {
  START_EVENT = 1,
  PROCESS_EVENT = 2,
  EMERGENCY_EVENT = 3
};

/////////////////////////////////////////////////////////////////
// Constants
const unsigned long DEMO_INTERVAL_MS = 3000;

/////////////////////////////////////////////////////////////////
SimpleFSM fsm;
unsigned long lastDemo = 0;
int demoStep = 0;

/////////////////////////////////////////////////////////////////
// State callback functions
void onIdleEnter() {
  Serial.println("=== IDLE STATE ===");
}

void onRunningEnter() {
  Serial.println("=== RUNNING STATE ===");
}

void onProcessingEnter() {
  Serial.println("=== PROCESSING STATE ===");
}

void onEmergencyEnter() {
  Serial.println("!!! EMERGENCY STATE !!!");
}

/////////////////////////////////////////////////////////////////
// Transition callback functions
void onEmergencyStop() {
  Serial.println(">>> EMERGENCY STOP: Global transition from any state!");
}

/////////////////////////////////////////////////////////////////
// Define states
State idleState("Idle", onIdleEnter);
State runningState("Running", onRunningEnter);
State processingState("Processing", onProcessingEnter);
State emergencyState("Emergency", onEmergencyEnter);

State* states[] = { &idleState, &runningState, &processingState, &emergencyState };

/////////////////////////////////////////////////////////////////
// Define regular transitions (between specific states)
Transition regularTransitions[] = {
  Transition(&idleState, &runningState, START_EVENT),
  Transition(&runningState, &processingState, PROCESS_EVENT)
};

/////////////////////////////////////////////////////////////////
// Define timed transitions
TimedTransition timedTransitions[] = {
  TimedTransition(&processingState, &idleState, 2000)
};

/////////////////////////////////////////////////////////////////
// Automated demo function
void runDemo() {
  if (millis() - lastDemo < DEMO_INTERVAL_MS) return;
  lastDemo = millis();
  
  Serial.println("\n--- Demo Step " + String(demoStep + 1) + " ---");
  
  switch (demoStep) {
    case 0:
      Serial.println("Triggering START_EVENT from idle state");
      fsm.trigger(START_EVENT);
      break;
      
    case 1:
      Serial.println("Triggering PROCESS_EVENT from running state");
      fsm.trigger(PROCESS_EVENT);
      break;
      
    case 2:
      Serial.println("Triggering EMERGENCY_EVENT (global transition from any state)");
      fsm.trigger(EMERGENCY_EVENT);
      break;
      
    case 3:
      Serial.println("Triggering START_EVENT again to restart the cycle");
      fsm.trigger(START_EVENT);
      break;
      
    default:
      Serial.println("Demo cycle complete - restarting demo sequence");
      demoStep = 0;
      break;
  }
  
  demoStep++;
}

/////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("\n" + String('=', 50));
  Serial.println("SimpleFSM Global Transitions Demo");
  Serial.println("Using NEW addGlobalTransition() helper functions");
  Serial.println(String('=', 50));
  
  // Setup FSM
  fsm.setInitialState(&idleState);
  
  // Add all states
  for (int i = 0; i < 4; i++) {
    fsm.addUniqueState(states[i]);
  }
  
  // Add regular transitions
  fsm.add(regularTransitions, 2);  
  // Global transition using helper function (works from ANY state)
  fsm.addGlobalTransition(&emergencyState, EMERGENCY_EVENT, onEmergencyStop);
  
  // Add timed transitions
  fsm.add(timedTransitions, 1);
  
  Serial.println("FSM initialized with:");
  Serial.println("- 2 regular transitions");
  Serial.println("- 1 global transition (NEW helper function)");
  Serial.println("- 1 timed transition");
  Serial.println("\nStarting automated demo...\n");
}

/////////////////////////////////////////////////////////////////
void loop() {
  fsm.run();
  runDemo();
  
  // Show current state periodically
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 5000) {
    Serial.println("Current state: " + fsm.getState()->getName());
    lastStatus = millis();
  }
}

/////////////////////////////////////////////////////////////////
