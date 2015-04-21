#ifndef UI_H
#define UI_H

#include "state_machine.h"

namespace ui {

struct UIState : ObjectState {
};

extern StateMachine<UIState> machine;

}  // namespace ui

#endif  // UI_H
