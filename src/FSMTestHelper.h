
#ifndef FSM_TEST_HELPER_H
#define FSM_TEST_HELPER_H

#include "SimpleFSM.h"

class FSMTestHelper {
public:
  // Expose changeToState for testing, with optional time argument
  static void changeToState(SimpleFSM& fsm, State* state, unsigned long now = 0) {
    fsm.changeToState(state, now);
  }
};

#endif
