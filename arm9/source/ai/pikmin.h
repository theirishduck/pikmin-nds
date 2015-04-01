#ifndef AI_PIKMIN_H
#define AI_PIKMIN_H

#include "state_machine.h"
#include "drawable_entity.h"

class Squad;

namespace pikmin_ai {

enum class PikminType {
  kRedPikmin,
  kYellowPikmin,
  kBluePikmin,
};

struct PikminState : ObjectState {
  PikminType type = PikminType::kRedPikmin;
  int id = 0;
  Squad* current_squad{nullptr};

  //variables related to locating nearby tasks
  int time_until_task_search = 300;

  //parent: used for being thrown and chewed
  DrawableEntity* parent{nullptr};
  Vec3 parent_initial_location;
  Vec3 child_offset;
};

extern StateMachine<PikminState> machine;

}  // namespace pikmin_ai

#endif