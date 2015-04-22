#ifndef AI_PIKMIN_H
#define AI_PIKMIN_H

#include "state_machine.h"
#include "drawable_entity.h"

namespace squad_ai {
struct SquadState;
}

namespace pikmin_ai {

enum class PikminType {
  kNone,
  kRedPikmin,
  kYellowPikmin,
  kBluePikmin,
};

struct PikminState : ObjectState {
  PikminType type = PikminType::kRedPikmin;
  int id = 0;
  bool active; // Used by the allocator to flag unused slots
  squad_ai::SquadState* current_squad{nullptr};

  //parent: used for being thrown and chewed
  DrawableEntity* parent{nullptr};
  Vec3 parent_initial_location;
  Vec3 child_offset;

  Vec2 target;

  //cache values for not updating so often
  numeric_types::Brads target_facing_angle;
};

extern StateMachine<PikminState> machine;

}  // namespace pikmin_ai

#endif