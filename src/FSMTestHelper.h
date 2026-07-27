
#ifndef FSM_TEST_HELPER_H
#define FSM_TEST_HELPER_H

#include "SimpleFSM.h"

class FSMTestHelper {
public:
  // Expose changeToState for testing, with optional time argument
  static void changeToState(SimpleFSM& fsm, State* state, unsigned long now = 0) {
    fsm.changeToState(state, now);
  }

  // Expose the run-timing check and its input so millis() rollover can be
  // exercised deterministically without waiting ~49 days.
  static void setLastRun(SimpleFSM& fsm, unsigned long value) {
    fsm.last_run = value;
  }
  static bool isTimeForRun(SimpleFSM& fsm, unsigned long now, int interval) {
    return fsm.isTimeForRun(now, interval);
  }
};

#endif
