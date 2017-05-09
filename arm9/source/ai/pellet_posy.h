#ifndef AI_POSY_H
#define AI_POSY_H

#include "state_machine.h"
#include "drawable_entity.h"
#include "physics/body.h"

#include "ai/health.h"

namespace posy_ai {

struct PosyState : ObjectState {
  health_ai::HealthState* health_state;
  unsigned int old_health;
  physics::Body* detection;
};

extern StateMachine<PosyState> machine;

}  // namespace posy_ai

#endif
