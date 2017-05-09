#ifndef AI_FIRE_SPOUT_H
#define AI_FIRE_SPOUT_H

#include "state_machine.h"
#include "drawable_entity.h"

#include "ai/health.h"

namespace fire_spout_ai {

struct FireSpoutState : ObjectState {
  physics::Body* flame_sensor;
  physics::Body* detection;
  int flame_timer{0};
  health_ai::HealthState* health_state;
};

extern StateMachine<FireSpoutState> machine;

}  // namespace fire_spout_ai

#endif
