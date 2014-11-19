#ifndef AI_PIKMIN_H
#define AI_PIKMIN_H

#include "state_machine.h"
#include "drawable_entity.h"

namespace pikmin_ai {

enum class PikminType {
  kRedPikmin,
  kYellowPikmin,
  kBluePikmin,
};

struct PikminState : ObjectState {
  PikminType type = PikminType::kRedPikmin;
};

extern StateMachine<PikminState> machine;

}  // namespace pikmin_ai

#endif