#include "treasure.h"

#include "dsgx.h"
#include "pikmin_game.h"
#include "physics/body.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

using pikmin_ai::PikminState;
using pikmin_ai::PikminType;
using physics::Body;

namespace treasure_ai {

bool TreasureState::AddPikmin(PikminState* pikmin) {
  if (RoomForMorePikmin()) {
    for (int i = 0; i < max_pikmin; i++) {
      if (active_pikmin[i] == nullptr) {
        active_pikmin[i] = pikmin;
        num_active_pikmin++;
        lift_timer = 0;
        return true;
      }
    }
  }
  return false;
}

void TreasureState::RemovePikmin(PikminState* pikmin) {
  for (int i = 0; i < max_pikmin; i++) {
    if (active_pikmin[i] == pikmin) {
      active_pikmin[i] = nullptr;
      num_active_pikmin--;
      lift_timer = 0;
      return;
    }
  }
}

bool TreasureState::RoomForMorePikmin() {
  return num_active_pikmin < max_pikmin;
}

bool TreasureState::Moving() {
  return num_active_pikmin >= cost;
}

void Init(TreasureState& treasure) {
  treasure.detection = treasure.entity->engine()->World().AllocateBody(&treasure);
  treasure.detection->position = treasure.entity->body()->position;
  treasure.detection->radius = 10_f;
  treasure.detection->height = 5_f;
  treasure.detection->is_sensor = true;
  treasure.detection->collision_group = DETECT_GROUP;
  treasure.detection->owner = treasure.entity->body();

  treasure.entity->body()->collision_group = TREASURE_GROUP;
  treasure.entity->body()->owner = &treasure;

  //initialize proper!
  for (int i = 0; i < 100; i++) {
    treasure.active_pikmin[i] = nullptr;
  }
}

int CountOfType(TreasureState& treasure, PikminType type) {
  int count = 0;
  for (int i = 0; i < treasure.max_pikmin; i++) {
    if (treasure.active_pikmin[i] != nullptr) {
      if (treasure.active_pikmin[i]->type == type) {
        count++;
      }
    }
  }
  if (type == PikminType::kRedPikmin) {
    debug::DisplayValue("REDS: ", count);
  }
  if (type == PikminType::kYellowPikmin) {
    debug::DisplayValue("YELLOWS: ", count);
  }
  if (type == PikminType::kBluePikmin) {
    debug::DisplayValue("BLUES: ", count);
  }
  return count;
}

void SetDestinationType(TreasureState& treasure) {
  int reds = CountOfType(treasure, PikminType::kRedPikmin);
  int yellows = CountOfType(treasure, PikminType::kYellowPikmin);
  int blues = CountOfType(treasure, PikminType::kBluePikmin);

  if (!(treasure.is_corpse)) {
    treasure.destination = DestinationType::kShip;
    return;
  }
  if (reds > yellows and
      reds > blues) {
    treasure.destination = DestinationType::kRedOnion;
    return;
  }
  if (yellows > reds and
      yellows > blues) {
    treasure.destination = DestinationType::kYellowOnion;
    return;
  }
  if (blues > reds and
      blues > yellows) {
    treasure.destination = DestinationType::kBlueOnion;
    return;
  }
  // Oh no! We can't make up our mind! Ah well, just pick one at random then.
  treasure.destination = (DestinationType)((rand() % 3) + 1);
}

void ClearDestinationType(TreasureState& treasure) {
  treasure.destination = DestinationType::kNone;
}

void UpdatePikminPositions(TreasureState& treasure) {
  debug::DisplayValue("ActivePikmin: ", treasure.num_active_pikmin);

  Brads clockwise_angle = 0_brad;
  Brads delta = 360_brad /
      fixed::FromInt(treasure.max_pikmin);
  for (int i = 0; i < treasure.max_pikmin; i++) {
    if (treasure.active_pikmin[i] != nullptr) {
      Body* pikmin_body = treasure.active_pikmin[i]->entity->body();
      pikmin_body->position.x = trig::CosLerp(clockwise_angle);
      pikmin_body->position.y = 0_f;
      pikmin_body->position.z = -trig::SinLerp(clockwise_angle);
      pikmin_body->position = pikmin_body->position * treasure.entity->body()->radius;
      pikmin_body->position += treasure.entity->body()->position;

      // Note: this bit here is going to be expensive. Maybe if it's a performance
      // issue, only rotate every so often? It shouldn't be super noticable if it
      // lags a bit.
      treasure.active_pikmin[i]->entity->RotateToFace(treasure.entity);
      clockwise_angle += delta;
    }
  }
}

void IdleAlways(TreasureState& treasure) {
  treasure.detection->position = treasure.entity->body()->position;
  UpdatePikminPositions(treasure);
  if (treasure.Moving()) {
    treasure.lift_timer++;
  }
  treasure.entity->body()->velocity = {0_f, 0_f, 0_f};
}

bool LiftTimerSatisfied(const TreasureState& treasure) {
  return treasure.lift_timer > 20;
}

bool LiftTimerReset(const TreasureState& treasure) {
  return treasure.lift_timer <= 20;
}

onion_ai::OnionState* DestinationBody(TreasureState& treasure) {
  if (treasure.destination == DestinationType::kRedOnion) {
    return treasure.game->Onion(PikminType::kRedPikmin);
  }
  if (treasure.destination == DestinationType::kYellowOnion) {
    return treasure.game->Onion(PikminType::kYellowPikmin);
  }
  if (treasure.destination == DestinationType::kBlueOnion) {
    return treasure.game->Onion(PikminType::kBluePikmin);
  }
  return nullptr;
}

void MoveTowardTarget(TreasureState& treasure) {
  auto destination = DestinationBody(treasure);
  auto new_velocity = destination->entity->body()->position - treasure.entity->body()->position;
  new_velocity.y = 0_f;
  new_velocity = new_velocity.Normalize() * 0.1_f;
  treasure.entity->body()->velocity = new_velocity;
  UpdatePikminPositions(treasure);
}

namespace TreasureNode {
enum TreasureNode {
  kInit = 0,
  kIdle,
  kMoving,
};
}

Edge<TreasureState> init[] {
  Edge<TreasureState>{kAlways, nullptr, Init, TreasureNode::kIdle},
  END_OF_EDGES(TreasureState)
};

Edge<TreasureState> idle[] {
  Edge<TreasureState>{kAlways, LiftTimerSatisfied, SetDestinationType, TreasureNode::kMoving},
  Edge<TreasureState>{kAlways, nullptr, IdleAlways, TreasureNode::kIdle},
  END_OF_EDGES(TreasureState)
};

Edge<TreasureState> moving[] {
  Edge<TreasureState>{kAlways, LiftTimerReset, ClearDestinationType, TreasureNode::kIdle},
  Edge<TreasureState>{kAlways, nullptr, MoveTowardTarget, TreasureNode::kMoving},
  END_OF_EDGES(TreasureState)
};

Node<TreasureState> node_list[] {
  {"init", true, init},
  {"idle", true, idle},
  {"moving", true, moving},
};

StateMachine<TreasureState> machine(node_list);

}  // namespace treasure_ai
