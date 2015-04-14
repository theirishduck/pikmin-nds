#ifndef AI_ONION_H
#define AI_ONION_H

#include "state_machine.h"
#include "drawable_entity.h"

namespace onion_ai {

struct OnionState : ObjectState {
  
};

extern StateMachine<OnionState> machine;

}  // namespace onion_ai

#endif
