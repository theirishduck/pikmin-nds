#include "treasure.h"

#include "dsgx.h"
#include "pikmin_game.h"
#include "ai/onion.h"
#include "physics/body.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

using pikmin_ai::PikminState;
using pikmin_ai::PikminType;
using physics::Body;

namespace treasure_ai {

void TreasureState::UpdateDetectionBody() {
  if (RoomForMorePikmin() and  !detection_active and carryable) {
    detection = world().AllocateBody(handle).body;
    detection->position = position();
    detection->radius = 10_f;
    detection->height = 5_f;
    detection->is_sensor = true;
    detection->collision_group = DETECT_GROUP;
    detection_active = true;
    return;
  }
  if (!RoomForMorePikmin() and detection_active) {
    // Disable the detection radius so pikmin stop attempting to chase us
    world().FreeBody(detection);
    detection_active = false;
    return;
  }
}

bool TreasureState::AddPikmin(PikminState* pikmin) {
  if (RoomForMorePikmin()) {
    for (int i = 0; i < max_pikmin; i++) {
      if (active_pikmin[i] == nullptr) {
        active_pikmin[i] = pikmin;
        num_active_pikmin++;
        lift_timer = 0;
        UpdateDetectionBody();
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
      UpdateDetectionBody();
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
  treasure.UpdateDetectionBody();

  treasure.body->collision_group = TREASURE_GROUP;

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
  treasure.game->DebugDictionary().Set("ActivePikmin: ", treasure.num_active_pikmin);

  Brads clockwise_angle = 0_brad;
  Brads delta = 360_brad /
      fixed::FromInt(treasure.max_pikmin);
  for (int i = 0; i < treasure.max_pikmin; i++) {
    if (treasure.active_pikmin[i] != nullptr) {
      Body* pikmin_body = treasure.active_pikmin[i]->body;
      pikmin_body->position.x = trig::CosLerp(clockwise_angle);
      pikmin_body->position.y = 0_f;
      pikmin_body->position.z = -trig::SinLerp(clockwise_angle);
      pikmin_body->position = pikmin_body->position * treasure.body->radius;
      pikmin_body->position += treasure.position();

      // Note: this bit here is going to be expensive. Maybe if it's a performance
      // issue, only rotate every so often? It shouldn't be super noticable if it
      // lags a bit.
      treasure.active_pikmin[i]->entity->RotateToFace(treasure.entity);
      clockwise_angle += delta;
    }
  }
}

void IdleAlways(TreasureState& treasure) {
  if (treasure.detection_active) {
    treasure.detection->position = treasure.position();
  }
  UpdatePikminPositions(treasure);
  if (treasure.Moving()) {
    treasure.lift_timer++;
  }
  treasure.set_velocity({0_f, 0_f, 0_f});
}

bool LiftTimerSatisfied(const TreasureState& treasure) {
  return treasure.lift_timer > 20;
}

bool LiftTimerReset(const TreasureState& treasure) {
  return treasure.lift_timer <= 20;
}

onion_ai::OnionState* DestinationBody(const TreasureState& treasure) {
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
  auto new_velocity = destination->position() - treasure.position();
  new_velocity.y = 0_f;
  new_velocity = new_velocity.Normalize() * 0.2_f;
  treasure.set_velocity(new_velocity);
  UpdatePikminPositions(treasure);
  if (treasure.detection_active) {
    treasure.detection->position = treasure.position();
  }
}

bool DestinationReached(const TreasureState& treasure) {
  auto destination = DestinationBody(treasure);
  auto distance = (destination->position() - treasure.position()).Length();
  return distance < 0.3_f;
}

void PrepareForRetrieval(TreasureState& treasure) {
  auto destination = DestinationBody(treasure);
  // Align with the destination region
  treasure.set_position(destination->position());
  // Kill the targeting radius, so pikmin stop trying to carry us
  if (treasure.detection_active) {
    treasure.world().FreeBody(treasure.detection);
    treasure.detection = nullptr;
  }
  // Dislodge all the pikmin carrying us
  treasure.carryable = false;
  // Remove ourselves from physics calculations, and prepare to rise into the
  // onion
  treasure.entity->body_handle().body->affected_by_gravity = false;
  treasure.set_velocity(Vec3{0_f, 0.2_f, 0_f});
}

void RiseIntoDestination(TreasureState& treasure) {
  auto scale = fixed::FromInt(60 - treasure.frames_at_this_node) / 60_f;
  treasure.entity->set_scale(scale);
}

void CollectTreasure(TreasureState& treasure) {
  treasure.dead = true;  // Goodbye, cruel world!
  if (treasure.destination == DestinationType::kRedOnion) {
    if (treasure.pikmin_affinity == PikminType::kYellowPikmin or
        treasure.pikmin_affinity == PikminType::kBluePikmin) {
      treasure.pikmin_seeds = treasure.pikmin_seeds / 2;
    }
    auto onion = treasure.game->Onion(PikminType::kRedPikmin);
    onion->seeds_count += treasure.pikmin_seeds;
  }
  if (treasure.destination == DestinationType::kYellowOnion) {
    if (treasure.pikmin_affinity == PikminType::kRedPikmin or
        treasure.pikmin_affinity == PikminType::kBluePikmin) {
      treasure.pikmin_seeds = treasure.pikmin_seeds / 2;
    }
    auto onion = treasure.game->Onion(PikminType::kYellowPikmin);
    onion->seeds_count += treasure.pikmin_seeds;
  }
  if (treasure.destination == DestinationType::kBlueOnion) {
    if (treasure.pikmin_affinity == PikminType::kRedPikmin or
        treasure.pikmin_affinity == PikminType::kYellowPikmin) {
      treasure.pikmin_seeds = treasure.pikmin_seeds / 2;
    }
    auto onion = treasure.game->Onion(PikminType::kBluePikmin);
    onion->seeds_count += treasure.pikmin_seeds;
  }
}

namespace TreasureNode {
enum TreasureNode {
  kInit = 0,
  kIdle,
  kMoving,
  kTractorBeam,
};
}

Edge<TreasureState> init[] {
  {Trigger::kAlways, nullptr, Init, TreasureNode::kIdle},
  END_OF_EDGES(TreasureState)
};

Edge<TreasureState> idle[] {
  {Trigger::kAlways, LiftTimerSatisfied, SetDestinationType, TreasureNode::kMoving},
  {Trigger::kAlways, nullptr, IdleAlways, TreasureNode::kIdle},
  END_OF_EDGES(TreasureState)
};

Edge<TreasureState> moving[] {
  {Trigger::kAlways, LiftTimerReset, ClearDestinationType, TreasureNode::kIdle},
  {Trigger::kAlways, DestinationReached, PrepareForRetrieval, TreasureNode::kTractorBeam},
  {Trigger::kAlways, nullptr, MoveTowardTarget, TreasureNode::kMoving},  // Loopback
  END_OF_EDGES(TreasureState)
};

Edge<TreasureState> tractor_beam[] {
  {Trigger::kLastFrame, nullptr, CollectTreasure, TreasureNode::kIdle}, // Destroy Self
  {Trigger::kAlways, nullptr, RiseIntoDestination, TreasureNode::kTractorBeam},  // Loopback
  END_OF_EDGES(TreasureState)
};

Node<TreasureState> node_list[] {
  {"init", true, init},
  {"idle", true, idle},
  {"moving", true, moving},
  {"tractor_beam", true, tractor_beam, nullptr, 60},
};

StateMachine<TreasureState> machine(node_list);

}  // namespace treasure_ai
