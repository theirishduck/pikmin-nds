#include "squad.h"
#include "pikmin.h"
#include "captain.h"
#include "trig.h"

#include <algorithm>

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

using pikmin_ai::PikminState;
using pikmin_ai::PikminType;

using namespace trig;

namespace squad_ai {

PikminState* SquadState::NextPikmin() {
  // Later: make this consider proximity to olimar perhaps?
  if (squad_size > 0) {
    return pikmin[0];
  }
  return nullptr;
}

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

void SquadState::SortPikmin(PikminType pikmin_type) {
  int pass = 0;
  int selected_color = (int)pikmin_type;

  int current_slot = 0;
  while (pass < 3) {
    int comparison_slot = current_slot + 1;
    while (comparison_slot < squad_size) {
      //loop current_slot until we find a pikmin that isn't the active type
      while ((int)pikmin[current_slot]->type == selected_color and current_slot < squad_size) {
        current_slot++;
      }
      //sanity: make sure comparison starts *after* the current slot
      if (comparison_slot <= current_slot) {
        comparison_slot = current_slot + 1;
      }
      //finally, loop comparison forward until we hit either a pikmin to swap
      //with (one that matches the current type) or the end of the list
      while(comparison_slot < squad_size and (int)pikmin[comparison_slot]->type != selected_color) {
        comparison_slot++;
      }
      //if we didn't run off the end of the list, perform the swap
      if (comparison_slot < squad_size) {
        std::swap(pikmin[current_slot], pikmin[comparison_slot]);
        current_slot++;
      }
      comparison_slot++;
    }

    selected_color++;
    if (selected_color > (int)PikminType::kBluePikmin) {
      selected_color = (int)PikminType::kRedPikmin;
    }
    pass++;
  }
}

int SquadState::PikminCount(PikminType pikmin_type) {
  int count = 0;
  for (int i = 0; i < squad_size; i++) {
    if (pikmin[i]->type == pikmin_type) {
      count++;
    }
  }
  return count;
}

void InitAlways(SquadState& squad) {
  squad.position = Vec3{64_f,0_f,-64_f};
}

const fixed kMaxDistanceFromCaptain = 10_f;
const fixed kSquadSpacing = 2.0_f;

void UpdateTestSquare(SquadState& squad) {
  // move ourselves close to the captain
  auto distance = (squad.captain->position() - squad.position).Length();
  if (distance > kMaxDistanceFromCaptain) {
    auto direction = squad.captain->position() - squad.position;
    direction = direction.Normalize() * kMaxDistanceFromCaptain;
    squad.position = squad.captain->position() - direction;
  }

  // easy pie! update all the pikmin targets in this squad
  for (int slot = 0; slot < squad.squad_size; slot++) {
    fixed x = (fixed::FromInt(slot % 10) - 4.5_f) * kSquadSpacing;
    fixed y = (fixed::FromInt(slot / 10) - 4.5_f) * kSquadSpacing;
    if ((slot / 10) % 2 == 0) {
      x *= -1_f;
    }
    squad.pikmin[slot]->target = Vec2{
      squad.position.x + x,
      squad.position.z + y // This is confusing!
    };
  }
}

// Note: might be useful to keep around as a C-stick shape, with some
// tweaking to values.
void UpdateTriangleShape(SquadState& squad) {
  if (squad.captain->held_pikmin) {
    squad.SortPikmin(squad.captain->held_pikmin->type);
  }

  // move ourselves close to the captain
  auto distance = (squad.captain->position() - squad.position).Length();
  if (distance > 3.0_f) {
    auto direction = squad.captain->position() - squad.position;
    direction = direction.Normalize() * 3.0_f;
    squad.position = squad.captain->position() - direction;
  }

  // loop through all the slots and assign a target position for each pikmin
  int slot = 0;
  int rank = 0;
  while (slot < squad.squad_size) {
    // This produces a triangle shape
    int rank_count = 1 + rank * 2;

    fixed rank_size = fixed::FromInt(rank_count - 1) * kSquadSpacing;
    Vec2 rank_start = {(rank_size * 0.5_f), fixed::FromInt(rank) * kSquadSpacing};
    Vec2 rank_delta = {-kSquadSpacing, 0_f};
    if (rank % 2 == 0) {
      // Every other frame, reverse the direction
      rank_start.x *= -1_f;
      rank_delta.x *= -1_f;
    }

    // rotate our start and our delta
    auto rot_angle = squad.captain->entity->AngleTo(squad.captain->cursor);
    rank_start = rank_start.Rotate(rot_angle);
    rank_delta = rank_delta.Rotate(rot_angle);

    Vec2 rank_pos = rank_start;
    int rank_index = 0;
    while (rank_index < rank_count and slot < squad.squad_size) {
      squad.pikmin[slot]->target = Vec2{
        squad.position.x + rank_pos.x,
        squad.position.z + rank_pos.y // This is confusing!
      };

      rank_pos += rank_delta;
      rank_index++;
      slot++;
    }
    rank++;
  }
}

void UpdateCircleShape(SquadState& squad) {
  if (squad.captain->held_pikmin) {
    squad.SortPikmin(squad.captain->held_pikmin->type);
  }

  // calculate the diameter of our circle
  fixed diameter = fixed::FromRaw(sqrtf32((fixed::FromInt(squad.squad_size) / 3.1415926_f).data_)) * 2_f;
  Brads stride = 180_brad / diameter * 0.85_f;

  // calculate the rotation for the squad
  if (squad.captain->held_pikmin) {
    squad.current_rotation = squad.captain->entity->AngleTo(squad.captain->cursor) + 90_brad;
    squad.position = squad.captain->position();
    // move the squad slightly behind olimar
    Vec2 delta = {0_f, 3_f};
    delta = delta.Rotate(squad.current_rotation);
    squad.position.x += delta.x;
    squad.position.z += delta.y;
  } else {
    // move ourselves close to the captain
    // use a modified squad position, based on our expected diameter
    auto squad_radius = (diameter * kSquadSpacing) / 2_f;
    Vec2 squad_center = {0_f, squad_radius};
    squad_center = squad_center.Rotate(squad.current_rotation);
    squad_center += Vec2{squad.position.x, squad.position.z};
    Vec2 captain_position = {squad.captain->position().x, squad.captain->position().z};
    auto distance = (captain_position - squad_center).Length();
    if (distance > squad_radius + 3_f) {
      //zap to the captain!
      auto offset = captain_position - squad_center;
      auto difference_delta = (distance - (squad_radius + 1_f)) / distance;
      offset *= difference_delta;
      squad.position.x += offset.x;
      squad.position.z += offset.y;
    }
  }

  // loop through all the slots and assign a target position for each pikmin
  int slot = 0;
  int rank = 0;
  while (slot < squad.squad_size) {
    // This produces a triangle shape
    int rank_count = (int)(trig::SinLerp(stride * (rank + 1)) * diameter);

    //sanity
    if (rank_count < 0) {
      rank_count *= -1;
    }
    rank_count += 1;

    fixed rank_size = fixed::FromInt(rank_count - 1) * kSquadSpacing;
    Vec2 rank_start = {(rank_size * 0.5_f), fixed::FromInt(rank) * kSquadSpacing};
    Vec2 rank_delta = {-kSquadSpacing, 0_f};
    if (rank % 2 == 0) {
      // Every other frame, reverse the direction
      rank_start.x *= -1_f;
      rank_delta.x *= -1_f;
    }

    // rotate our start and our delta
    rank_start = rank_start.Rotate(squad.current_rotation);
    rank_delta = rank_delta.Rotate(squad.current_rotation);

    Vec2 rank_pos = rank_start;
    int rank_index = 0;
    while (rank_index < rank_count and slot < squad.squad_size) {
      squad.pikmin[slot]->target = Vec2{
        squad.position.x + rank_pos.x,
        squad.position.z + rank_pos.y // This is confusing!
      };

      rank_pos += rank_delta;
      rank_index++;
      slot++;
    }
    rank++;
  }
}

Edge<SquadState> init[] {
  {Trigger::kAlways, nullptr, InitAlways, 1},
  END_OF_EDGES(SquadState)
};

Edge<SquadState> circle_following_captain[] {
  {Trigger::kAlways, nullptr, UpdateCircleShape, 1},
  END_OF_EDGES(SquadState)
};

Node<SquadState> node_list[] {
  {"Init", true, init},
  {"UpdateAlways", true, circle_following_captain},
};

StateMachine<SquadState> machine(node_list);

} // namespace squad_ai
