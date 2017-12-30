#ifndef DEBUG_UI_H
#define DEBUG_UI_H

#include "ai/pikmin_game_state.h"

class PikminGame;

namespace debug_ui {

struct DebugUiState : PikminGameState {
  std::map<std::string, std::function<PikminGameState*(PikminGame*)> >::const_iterator current_spawner;
  std::vector<std::string> level_names;
  u8 current_level{0};
};

extern StateMachine<DebugUiState> machine;

}  // namespace debug_ui

#endif  // DEBUG_UI_H
