#ifndef UI_H
#define UI_H

#include "state_machine.h"

namespace ui {

struct UIState : ObjectState {
  int pikmin_delta;
  int key_timer = 0;
  int touch_timer = 0;
};

extern StateMachine<UIState> machine;

}  // namespace ui

#endif  // UI_H
