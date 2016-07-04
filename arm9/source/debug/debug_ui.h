#ifndef DEBUG_UI_H
#define DEBUG_UI_H

#include "state_machine.h"

class PikminGame;

namespace debug_ui {

struct DebugUiState : ObjectState {
  std::map<std::string, std::function<ObjectState*(PikminGame*)> >::const_iterator current_spawner;
};

extern StateMachine<DebugUiState> machine;

}  // namespace debug_ui

#endif  // DEBUG_UI_H
