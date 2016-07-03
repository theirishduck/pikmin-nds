#ifndef UI_H
#define UI_H

#include "state_machine.h"
#include "debug_ui.h"

namespace ui {

struct UIState : ObjectState {
  int pikmin_delta;
  int key_timer = 0;
  int touch_timer = 0;

  debug_ui::DebugUiState debug_state;
};

extern StateMachine<UIState> machine;

}  // namespace ui

#endif  // UI_H
