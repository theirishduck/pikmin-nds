#ifndef AI_FIRE_SPOUT_H
#define AI_FIRE_SPOUT_H

#include "ai/pikmin_game_state.h"

#include "ai/health.h"

namespace fire_spout_ai {

struct FireSpoutState : PikminGameState {
  physics::Body* flame_sensor;
  physics::Body* detection;
  int flame_timer{0};
  health_ai::HealthState* health_state;
};

extern StateMachine<FireSpoutState> machine;

}  // namespace fire_spout_ai

#endif
