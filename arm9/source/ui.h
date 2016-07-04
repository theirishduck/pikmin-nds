#ifndef UI_H
#define UI_H

#include "state_machine.h"
#include "debug/debug_ui.h"

namespace ui {

struct UIState : ObjectState {
  int pikmin_delta;
  int key_timer = 0;
  int touch_timer = 0;

  debug_ui::DebugUiState debug_state;
  bool debug_screen_active = false;
  int debug_topic_id;
};

extern StateMachine<UIState> machine;

}  // namespace ui

#endif  // UI_H
