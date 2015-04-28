#ifndef AI_SQUAD_H
#define AI_SQUAD_H

#include "state_machine.h"

namespace pikmin_ai {
struct PikminState;
enum class PikminType;
}

namespace captain_ai {
struct CaptainState;
}

namespace squad_ai {

struct SquadState : ObjectState {
  captain_ai::CaptainState* captain;
  pikmin_ai::PikminState* pikmin[100];
  int squad_size{0};

  Vec3 position;
  numeric_types::Brads current_rotation;

  void AddPikmin(pikmin_ai::PikminState* pikmin);
  void RemovePikmin(pikmin_ai::PikminState* pikmin);
  void SortPikmin(pikmin_ai::PikminType pikmin_type);
  pikmin_ai::PikminState* NextPikmin();
};

extern StateMachine<SquadState> machine;

}  // namespace squad_ai

#endif