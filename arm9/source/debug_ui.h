#ifndef DEBUG_UI_H
#define DEBUG_UI_H

#include "state_machine.h"

namespace debug_ui {

struct DebugUiState : ObjectState {
  
};

extern StateMachine<DebugUiState> machine;

}  // namespace debug_ui

#endif  // DEBUG_UI_H
