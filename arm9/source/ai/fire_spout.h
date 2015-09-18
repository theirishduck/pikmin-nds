#ifndef AI_FIRE_SPOUT_H
#define AI_FIRE_SPOUT_H

#include "state_machine.h"
#include "drawable_entity.h"

namespace fire_spout_ai {

struct FireSpoutState : ObjectState {
  physics::Body* flame_sensor;
  physics::Body* detection;
  int flame_timer{0};
  int health{50};
  bool active{true};
};

extern StateMachine<FireSpoutState> machine;

}  // namespace onion_ai

#endif
