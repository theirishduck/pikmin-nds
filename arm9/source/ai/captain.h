#ifndef AI_CAPTAIN_H
#define AI_CAPTAIN_H

#include "state_machine.h"
#include "drawable_entity.h"

namespace captain_ai {

struct CaptainState : ObjectState {
  numeric_types::Brads current_angle = numeric_types::Brads::Raw(0);
};

extern StateMachine<CaptainState> machine;

}  // namespace captain_ai

#endif