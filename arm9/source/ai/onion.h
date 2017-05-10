#ifndef AI_ONION_H
#define AI_ONION_H

#include "ai/pikmin_game_state.h"
#include "ai/pikmin.h"

namespace onion_ai {

struct OnionState : PikminGameState {
  physics::Body* feet[3];
  pikmin_ai::PikminType pikmin_type = pikmin_ai::PikminType::kRedPikmin;
  int withdraw_count{0};
  int seeds_count{0};
  int old_seeds_count_{0};
};

extern StateMachine<OnionState> machine;

}  // namespace onion_ai

#endif
