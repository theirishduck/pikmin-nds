#ifndef AI_TREASURE_H
#define AI_TREASURE_H

#include "state_machine.h"
#include "drawable_entity.h"

namespace treasure_ai {

  enum class DestinationType {
    kNone,
    kRedOnion,
    kYellowOnion,
    kBlueOnion,
    kShip
  };

struct TreasureState : ObjectState {
  int cost{10};
  int max_pikmin{15};
  DestinationType destination{DestinationType::kNone};
  int lift_timer{0};
  bool is_corpse{true}; // Corpses go to onions, treasures go to the ship
  bool active;
};

extern StateMachine<TreasureState> machine;

}  // namespace treasure_ai

#endif
