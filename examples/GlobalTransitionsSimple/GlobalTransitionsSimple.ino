/////////////////////////////////////////////////////////////////
/*
  GlobalTransitionsSimple.ino
  
  This simplified example demonstrates the NEW addGlobalTransition() helper 
  functions in SimpleFSM. Global transitions work from ANY state and are 
  perfect for emergency stops and system-wide events.
  
  This example shows:
  - NEW: fsm.addGlobalTransition() helper function (recommended)
  - OLD: Transition(NULL, state, event) approach (commented out)
  
  The NEW approach is cleaner, more explicit, and easier to understand.
  
  This example works on all Arduino platforms without external dependencies.
*/
/////////////////////////////////////////////////////////////////

#include "SimpleFSM.h"

/////////////////////////////////////////////////////////////////
// Event definitions
enum Events {
  START_EVENT = 1,
  PROCESS_EVENT = 2,
  EMERGENCY_EVENT = 3,
  RESET_EVENT = 4
};

/////////////////////////////////////////////////////////////////
// Constants
const unsigned long DEMO_INTERVAL_MS = 3000;
const unsigned long AUTO_TIMEOUT_MS = 8000;

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

void onSystemReset() {
  Serial.println(">>> SYSTEM RESET: Global transition back to idle");
}

void onAutoShutdown() {
  Serial.println(">>> AUTO SHUTDOWN: Global timeout triggered");
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
// Define GLOBAL transitions - OLD WAY (commented out for comparison)
// This shows the traditional approach using NULL as source state:
/*
Transition globalTransitions[] = {
  // Emergency stop - can be triggered from ANY state
  Transition(NULL, &emergencyState, EMERGENCY_EVENT, onEmergencyStop)
};
*/

// Compare:
//   OLD: Transition(NULL, &emergencyState, EMERGENCY_EVENT, callback)
//   NEW: fsm.addGlobalTransition(&emergencyState, EMERGENCY_EVENT, callback)

/////////////////////////////////////////////////////////////////
// Define timed transitions
TimedTransition timedTransitions[] = {
  // Regular timed transition: Processing -> Idle after 2 seconds
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
      
    case 4:
      Serial.println("Triggering PROCESS_EVENT to go to processing state");
      fsm.trigger(PROCESS_EVENT);
      Serial.println("Processing will auto-return to idle after 2 seconds (timed transition)");
      break;
      
    default:
      Serial.println("Demo cycle complete - restarting demo sequence");
      demoStep = 0;  // Reset demo to start over
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
  
  // Add global transitions using NEW helper functions (recommended approach)
  // This replaces the old way: 
  // Transition(NULL, &emergencyState, EMERGENCY_EVENT, onEmergencyStop)
  
  fsm.addGlobalTransition(&emergencyState, EMERGENCY_EVENT, onEmergencyStop);
  
  // Add timed transitions
  fsm.add(timedTransitions, 1);
  
  Serial.println("FSM initialized with:");
  Serial.println("- 2 regular transitions");
  Serial.println("- 1 global transition (NEW helper function)");
  Serial.println("- 1 timed transition");
  Serial.println("\nComparison:");
//   Serial.println("OLD: Transition(NULL, &emergencyState, EMERGENCY_EVENT, callback)");
  Serial.println("NEW: fsm.addGlobalTransition(&emergencyState, EMERGENCY_EVENT, callback)");
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
