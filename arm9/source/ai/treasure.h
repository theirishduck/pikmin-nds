#ifndef AI_TREASURE_H
#define AI_TREASURE_H

#include "state_machine.h"
#include "drawable_entity.h"
#include "physics/body.h"
#include "pikmin.h"

namespace treasure_ai {

  enum class DestinationType {
    kNone,
    kRedOnion,
    kYellowOnion,
    kBlueOnion,
    kShip
  };

struct TreasureState : ObjectState {
  int cost{3};
  int max_pikmin{3};
  DestinationType destination{DestinationType::kNone};
  int lift_timer{0};
  bool is_corpse{true}; // Corpses go to onions, treasures go to the ship
  bool carryable{true};
  int pokos{0}; // Treasure worth, in pokos
  int pikmin_seeds{10};  // Treasure worth, in pikmin seeds
  pikmin_ai::PikminType pikmin_affinity{pikmin_ai::PikminType::kNone};


  physics::Body* detection;
  pikmin_ai::PikminState* active_pikmin[100];
  int num_active_pikmin{0};

  bool AddPikmin(pikmin_ai::PikminState* pikmin);
  void RemovePikmin(pikmin_ai::PikminState* pikmin);
  bool RoomForMorePikmin();
  bool Moving();
};

extern StateMachine<TreasureState> machine;

}  // namespace treasure_ai

#endif
