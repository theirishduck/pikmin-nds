#include "squad.h"
#include "pikmin.h"
#include "captain.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

using pikmin_ai::PikminState;

namespace squad_ai {

void SquadState::AddPikmin(PikminState* new_pikmin) {
  pikmin[squad_size] = new_pikmin;
  new_pikmin->current_squad = this;
  squad_size++;
}

void SquadState::RemovePikmin(PikminState* old_pikmin) {
  int slot = 0;
  while (pikmin[slot] != old_pikmin and slot < 100) {
    slot++;
  }

  //sanity check; does this pikmin actually exist in this squad?
  if (100 <= slot) {
    return;
  }  
  old_pikmin->current_squad = nullptr;
  pikmin[slot] = nullptr;

  //Shift the remaining members down to fill in the gap
  while (slot < 99) {
    pikmin[slot] = pikmin[slot + 1];
    slot++;
  }
  squad_size--;
}

void InitAlways(SquadState& squad) {
  squad.position = Vec3{64_f,0_f,-64_f};
}

const fixed kMaxDistanceFromCaptain = 10_f;

void UpdateTestShape(SquadState& squad) {
  // move ourselves close to the captain
  auto distance = (squad.captain->entity->body()->position - squad.position).Length();
  if (distance > kMaxDistanceFromCaptain) {
    auto direction = squad.captain->entity->body()->position - squad.position;
    direction = direction.Normalize() * kMaxDistanceFromCaptain;
    squad.position = squad.captain->entity->body()->position - direction;
  }

  // easy pie! update all the pikmin targets in this squad
  for (int slot = 0; slot < squad.squad_size; slot++) {
    fixed x = (fixed::FromInt(slot % 10) - 4.5_f) * 2.5_f;
    fixed y = (fixed::FromInt(slot / 10) - 4.5_f) * 2.5_f;
    if ((slot / 10) % 2 == 0) {
      x *= -1_f;
    }
    squad.pikmin[slot]->target = Vec2{
      squad.position.x + x,
      squad.position.z + y // This is confusing!
    };
  }
}

Edge<SquadState> edge_list[] {
  // Init
  Edge<SquadState>{kAlways, nullptr, InitAlways, 1},

  // Test / Follow Captain thing
  Edge<SquadState>{kAlways, nullptr, UpdateTestShape, 1},  
};

Node node_list[] {
  {"Init", true, 0, 0},
  {"UpdateAlways", true, 1, 1},
};

StateMachine<SquadState> machine(node_list, edge_list);

} // namespace squad_ai