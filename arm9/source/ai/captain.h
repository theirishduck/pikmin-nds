#ifndef AI_CAPTAIN_H
#define AI_CAPTAIN_H

#include "ai/pikmin_game_state.h"
#include "pikmin.h"
#include "onion.h"
#include "squad.h"

namespace physics {
  struct Body;
}

namespace captain_ai {

struct CaptainState : PikminGameState {
  numeric_types::Brads current_angle = numeric_types::Brads::Raw(0);
  pikmin_ai::PikminState* held_pikmin;
  DrawableEntity* cursor;
  DrawableEntity* whistle;
  physics::Body* whistle_body;
  physics::Body* cursor_body;
  int whistle_timer = 0;
  squad_ai::SquadState squad;

  onion_ai::OnionState* active_onion = nullptr;
};

extern StateMachine<CaptainState> machine;

}  // namespace captain_ai

#endif
