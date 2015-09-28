#ifndef AI_STATIC_H
#define AI_STATIC_H

#include "state_machine.h"
#include "drawable_entity.h"

namespace static_ai {

struct StaticState : ObjectState {
};

extern StateMachine<StaticState> machine;

}  // namespace static_ai

#endif
