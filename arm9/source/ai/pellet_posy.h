#ifndef AI_POSY_H
#define AI_POSY_H

#include "ai/health.h"
#include "ai/pikmin_game_state.h"
#include "physics/body.h"

namespace posy_ai {

struct PosyState : PikminGameState {
  health_ai::HealthState* health_state;
  unsigned int old_health;
  physics::Body* detection;
};

extern StateMachine<PosyState> machine;

}  // namespace posy_ai

#endif
