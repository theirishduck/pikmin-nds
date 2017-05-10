#ifndef AI_STATIC_H
#define AI_STATIC_H

#include "ai/pikmin_game_state.h"

namespace static_ai {

struct StaticState : PikminGameState {
};

extern StateMachine<StaticState> machine;

}  // namespace static_ai

#endif
