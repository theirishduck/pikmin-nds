#include "pikmin.h"
#include "captain.h"
#include "onion.h"
#include "treasure.h"
#include "pikmin_game.h"

#include "dsgx.h"
#include "multipass_engine.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

using treasure_ai::TreasureState;

namespace pikmin_ai {

const fixed kRunSpeed = 40.0_f / 60_f;
const fixed kTargetThreshold = 2.0_f;

TreasureState* GetActiveTreasure(const PikminState& pikmin) {
  TreasureState* treasure = (TreasureState*) pikmin.chase_target->owner;
  return treasure;
}

void InitAlways(PikminState& pikmin) {
  switch (pikmin.type) {
    case PikminType::kNone:
    case PikminType::kRedPikmin:
      pikmin.entity->set_actor(pikmin.game->ActorAllocator()->Retrieve("pikmin"));
      pikmin.entity->set_mesh("red_pikmin");
      break;
    case PikminType::kYellowPikmin:
      pikmin.entity->set_actor(pikmin.game->ActorAllocator()->Retrieve("pikmin"));
      pikmin.entity->set_mesh("yellow_pikmin");
      break;
    case PikminType::kBluePikmin:
      pikmin.entity->set_actor(pikmin.game->ActorAllocator()->Retrieve("pikmin"));
      pikmin.entity->set_mesh("blue_pikmin");
      break;
  }

  auto body = pikmin.entity->body();
  body->height = 6_f;
  body->radius = 1.0_f;

  body->collides_with_bodies = 1;
  body->is_pikmin = 1;
  body->is_movable = 1;
  body->sensor_groups = WHISTLE_GROUP | DETECT_GROUP | ATTACK_GROUP | TREASURE_GROUP;

  pikmin.entity->important = false;

  pikmin.current_node = pikmin.starting_state;
}

void IdleAlways(PikminState& pikmin) {
  if (pikmin.current_squad) {
    //every 8 frames or so, update our facing direction to look at the captain
    if ((pikmin.entity->engine()->FrameCounter() + pikmin.id) % 8 == 0) {
      pikmin.target_facing_angle = pikmin.entity->AngleTo(pikmin.current_squad->captain->entity);
    }
    pikmin.entity->RotateToFace(pikmin.target_facing_angle, 10_brad);
  }
}

// This function, assuming unique Pikmin IDs, will return true once every
// 100 frames or so, and will only return true for one pikmin on any given
// frame. This is very handy for making sure that very complex AI tasks aren't
// happening too much in a single frame.
bool AiStaggerDelay(const PikminState& pikmin) {
  return (pikmin.entity->engine()->FrameCounter() % 100) == (pikmin.id % 100);
}

bool HasNewParent(const PikminState& pikmin) {
  return pikmin.parent != nullptr;
}

void StoreParentLocation(PikminState& pikmin) {
  if (pikmin.parent) {
    pikmin.parent_initial_location = pikmin.parent->body()->position;
    pikmin.child_offset = pikmin.parent_initial_location
        - pikmin.entity->body()->position;
  }
  pikmin.entity->body()->velocity = Vec3{0_f,0_f,0_f};
}

void FollowParent(PikminState& pikmin) {
  if (pikmin.parent) {
    pikmin.entity->body()->position = pikmin.parent->body()->position
        + pikmin.child_offset;
  }
}

bool LeftParent(const PikminState& pikmin) {
  return pikmin.parent == nullptr;
}

void StopMoving(PikminState& pikmin) {
  pikmin.entity->body()->velocity = Vec3{0_f, 0_f, 0_f};
}

void ClearTargetAndStop(PikminState& pikmin) {
  pikmin.has_target = false;
  StopMoving(pikmin);
}

bool Landed(const PikminState& pikmin) {
  return pikmin.entity->body()->touching_ground;
}

void FaceTarget(PikminState& pikmin) {
  auto body = pikmin.entity->body();
  Vec2 posXZ{body->position.x, body->position.z};
  Vec2 random_offset = Vec2{
    fixed::FromInt(rand() % 10) / 5_f - 0.5_f,
    fixed::FromInt(rand() % 10) / 5_f - 0.5_f,
  };
  Vec2 new_direction = (pikmin.target + random_offset - posXZ).Normalize();
  fixed movement_speed = ((pikmin.target + random_offset - posXZ).Length() / 4_f);
  if (movement_speed > kRunSpeed) {
    movement_speed = kRunSpeed;
  }
  Vec2 new_velocity = new_direction * movement_speed;
  body->velocity.x = new_velocity.x;
  body->velocity.z = new_velocity.y;
  pikmin.entity->RotateToXZDirection(new_velocity);
}

void RunToTarget(PikminState& pikmin) {
  // Only update the angle every so often, as this is expensive!
  if ((pikmin.id + pikmin.entity->engine()->FrameCounter()) % 4 == 0) {
    FaceTarget(pikmin);
  }
}

bool PikminTurn(const PikminState& pikmin) {
  return pikmin.entity->engine()->FrameCounter() % 100 == pikmin.id;
}

template <int Chance>
bool RandomTurnChance(const PikminState& pikmin) {
  return PikminTurn(pikmin) and rand() % 100 < Chance;
}

void ChooseRandomTarget(PikminState& pikmin) {
  auto body = pikmin.entity->body();
  Vec2 new_target{body->position.x, body->position.z};
  new_target.x += fixed::FromInt((rand() % 30) - 15);
  new_target.y += fixed::FromInt((rand() % 30) - 15);
  pikmin.target = new_target;
}

bool TargetReached(const PikminState& pikmin) {
  //don't do this every frame, for intentional inaccuracy
  if ((pikmin.id + pikmin.entity->engine()->FrameCounter()) % 16 == 0) {
    auto position = pikmin.entity->body()->position;
    return (pikmin.target - Vec2{position.x, position.z}).Length2() <
        kTargetThreshold * kTargetThreshold;
  }
  return false;
}

bool CantReachTarget(const PikminState& pikmin) {
  return false; // STUB
}

bool TooFarFromTarget(const PikminState& pikmin) {
  // Do we have a valid target?
  if (pikmin.current_squad == nullptr and !pikmin.has_target) {
    return false;
  }

  //Are we too far away from our squad's set position?
  auto position = pikmin.entity->body()->position;
  return (pikmin.target - Vec2{position.x, position.z}).Length2() >
      kTargetThreshold * kTargetThreshold;
}

bool CollidedWithWhistle(const PikminState& pikmin) {
  if (pikmin.current_squad == nullptr) {
    if (pikmin.entity->body()->result_groups & WHISTLE_GROUP) {
      return true;
    }
  }
  return false;
}

void JoinSquad(PikminState& pikmin) {
  auto result = pikmin.entity->body()->FirstCollisionWith(WHISTLE_GROUP);
  // make sure we got a real result (this can fail in extreme cases)
  if (result.body) {
    auto captain = (captain_ai::CaptainState*)result.body->owner;
    pikmin.current_squad = &captain->squad;
    captain->squad.AddPikmin(&pikmin);
  }
}

bool ChaseTargetInvalid(const PikminState& pikmin) {
  // Some unspeakable horror caused our target to vanish or otherwise change
  if (!pikmin.chase_target or !pikmin.chase_target->active or !pikmin.chase_target->owner) {
    return true;
  }

  // Treasure has no more room for us... :(
  if (pikmin.chase_target->collision_group & TREASURE_GROUP) {
    auto treasure = GetActiveTreasure(pikmin);
    if (!(treasure->RoomForMorePikmin())) {
      return true;
    }
  }
  // TODO: Expand on this a bit more. Identifiers / handles?

  return false;
}

bool CollideWithAttackable(const PikminState& pikmin) {
  if (pikmin.entity->body()->result_groups & ATTACK_GROUP) {
    return true;
  }
  return false;
}

void ChaseTarget(PikminState& pikmin) {
  pikmin.target = Vec2{pikmin.chase_target->position.x, pikmin.chase_target->position.z};
  RunToTarget(pikmin);
}

void DealDamageToTarget(PikminState& pikmin) {
  int* target_health = (int*)pikmin.chase_target->owner;
  if (target_health) {
    *target_health -= 5;
  }
}

void JumpTowardTarget(PikminState& pikmin) {
  // Face the target, then apply upwards velocity
  FaceTarget(pikmin);
  pikmin.entity->body()->velocity.y = 0.4_f;
}

bool CollideWithTarget(const PikminState& pikmin) {
  if (pikmin.current_squad) {
    return false;
  }
  if (pikmin.entity->body()->result_groups & DETECT_GROUP) {
    auto target_circle = pikmin.entity->body()->FirstCollisionWith(DETECT_GROUP);
    if (target_circle.body) {
      if (target_circle.body->owner) {
        return true;
      }
    }
  }
  return false;
}

void StoreTargetBody(PikminState& pikmin) {
  auto target_circle = pikmin.entity->body()->FirstCollisionWith(DETECT_GROUP);
  if (target_circle.body) {
    pikmin.chase_target = (physics::Body*)target_circle.body->owner;
  }
}

void Aim(PikminState& pikmin) {
  FaceTarget(pikmin);
  pikmin.entity->body()->velocity = Vec3{0_f,0_f,0_f};
}

bool CollideWithOnionFoot(const PikminState& pikmin) {
  if (pikmin.entity->body()->result_groups & ONION_FEET_GROUP) {
    return true;
  }
  return false;
}

void StartClimbingOnion(PikminState& pikmin) {
  // Grab the onion / foot that we're targeting
  auto onion_foot = pikmin.entity->body()->FirstCollisionWith(ONION_FEET_GROUP);
  fixed travel_frames = 60_f;
  if (onion_foot.body) {
    auto onion = (onion_ai::OnionState*)onion_foot.body->owner;
    auto pikmin_body = pikmin.entity->body();
    pikmin_body->position = onion_foot.body->position;
    pikmin_body->affected_by_gravity = false;
    pikmin_body->collision_group = 0;
    pikmin_body->sensor_groups = 0;
    pikmin_body->is_movable = 0;

    pikmin.entity->RotateToFace(onion->entity);
    auto onion_position = onion->entity->body()->position;
    Vec2 climb_xz = (
        Vec2{onion_position.x, onion_position.z} -
        Vec2{pikmin_body->position.x, pikmin_body->position.z}
      ).Normalize();
    climb_xz = climb_xz * 9.2_f;
    pikmin.entity->body()->velocity = Vec3{
      climb_xz.x / travel_frames,
      9.1_f / travel_frames,
      climb_xz.y / travel_frames
    };
  }
}

void EnterOnion(PikminState& pikmin) {
  pikmin.dead = true; // Goodbye, pikmin!
  if (pikmin.type == PikminType::kRedPikmin) {
    pikmin.game->CurrentSaveData()->red_pikmin++;
  }
  if (pikmin.type == PikminType::kYellowPikmin) {
    pikmin.game->CurrentSaveData()->yellow_pikmin++;
  }
  if (pikmin.type == PikminType::kBluePikmin) {
    pikmin.game->CurrentSaveData()->blue_pikmin++;
  }
}

void WhistleOffOnion(PikminState& pikmin) {
  pikmin.entity->body()->affected_by_gravity = true;
  pikmin.game->ActiveCaptain()->squad.AddPikmin(&pikmin);
  pikmin.entity->body()->velocity.y = 0.5_f;
}

void HopOffFoot(PikminState& pikmin) {
  pikmin.entity->body()->affected_by_gravity = true;
  pikmin.entity->body()->velocity.y = 1.0_f;

  pikmin.game->ActiveCaptain()->squad.AddPikmin(&pikmin);
}

bool CollideWithValidTreasure(const PikminState& pikmin) {
  if (pikmin.entity->body()->result_groups & TREASURE_GROUP) {
    auto treasure_result = pikmin.entity->body()->FirstCollisionWith(TREASURE_GROUP);
    if (treasure_result.body != nullptr) {
      auto treasure = (TreasureState*)treasure_result.body->owner;
      if (treasure->RoomForMorePikmin()) {
        return true;
      }
    }
  }
  return false;
}

void AddToTreasure(PikminState& pikmin) {
  auto treasure = GetActiveTreasure(pikmin);
  treasure->AddPikmin(&pikmin);
  StopMoving(pikmin);
}

void RemoveFromTreasure(PikminState& pikmin) {
  auto treasure = GetActiveTreasure(pikmin);
  treasure->RemovePikmin(&pikmin);
}

void WhistleOffTreasure(PikminState& pikmin) {
  auto treasure = GetActiveTreasure(pikmin);
  treasure->RemovePikmin(&pikmin);
  JoinSquad(pikmin);
}

bool TreasureMoving(const PikminState& pikmin) {
  auto treasure = GetActiveTreasure(pikmin);
  return treasure->Moving();
}

bool TreasureInvalid(const PikminState& pikmin) {
  auto treasure = GetActiveTreasure(pikmin);
  if (treasure == nullptr) {
    return true;
  }
  return false;
}

bool TreasureStopped(const PikminState& pikmin) {
  return !TreasureMoving(pikmin);
}


Edge<PikminState> init[] {
  // Init
  Edge<PikminState>{kAlways, nullptr, InitAlways, PikminNode::kIdle},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> idle[] {
  // Idle
  {kAlways, CollideWithOnionFoot, StartClimbingOnion, PikminNode::kClimbIntoOnion},
  {kAlways, TooFarFromTarget, nullptr, PikminNode::kTargeting},
  {kAlways, CollidedWithWhistle, JoinSquad, PikminNode::kIdle},
  {kAlways, HasNewParent, StoreParentLocation, PikminNode::kGrabbed},
  {kAlways, CollideWithTarget, StoreTargetBody, PikminNode::kChasing},
  {kAlways,nullptr,IdleAlways,PikminNode::kIdle}, // Loopback
  END_OF_EDGES(PikminState)
};

Edge<PikminState> grabbed[] {
  // Grabbed
  {kAlways, LeftParent, nullptr, PikminNode::kThrown},
  {kAlways, nullptr, FollowParent, PikminNode::kGrabbed},  // Loopback
  END_OF_EDGES(PikminState)
};

Edge<PikminState> thrown[] {
  // Thrown
  {kAlways, Landed, StopMoving, PikminNode::kIdle},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> targeting[] {
  // Targeting
  {kAlways, CollideWithOnionFoot, StartClimbingOnion, PikminNode::kClimbIntoOnion},
  {kAlways, TargetReached, ClearTargetAndStop, PikminNode::kIdle},
  {kAlways, CantReachTarget, ClearTargetAndStop, PikminNode::kIdle},
  {kAlways, HasNewParent, StoreParentLocation, PikminNode::kGrabbed},
  {kAlways, nullptr, RunToTarget, PikminNode::kTargeting},  // loopback
  END_OF_EDGES(PikminState)
};

Edge<PikminState> chasing[] {
  // Chasing (Attack, Work, Carry)
  {kAlways, CantReachTarget, ClearTargetAndStop, PikminNode::kIdle},
  {kAlways, ChaseTargetInvalid, ClearTargetAndStop, PikminNode::kIdle},
  {kAlways, CollidedWithWhistle, JoinSquad, PikminNode::kIdle},
  {kAlways, CollideWithAttackable, StopMoving, PikminNode::kStandingAttack},
  {kAlways, CollideWithValidTreasure, AddToTreasure, PikminNode::kLiftTreasure},
  {kAlways, nullptr, ChaseTarget, PikminNode::kChasing},  // loopback
  END_OF_EDGES(PikminState)
};

Edge<PikminState> standing_attack[] {
  // Standing Attack
  {kAlways, ChaseTargetInvalid, ClearTargetAndStop, PikminNode::kIdle},
  {kFirstFrame, nullptr, Aim, PikminNode::kStandingAttack},
  {kLastFrame, nullptr, DealDamageToTarget, PikminNode::kJump},
  {kAlways, CollidedWithWhistle, JoinSquad, PikminNode::kIdle},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> jumping[] {
  // Jump
  {kAlways, ChaseTargetInvalid, ClearTargetAndStop, PikminNode::kIdle},
  {kFirstFrame, nullptr, JumpTowardTarget, PikminNode::kJump},
  {kAlways, CollidedWithWhistle, JoinSquad, PikminNode::kIdle},
  {kAlways, Landed, StopMoving, PikminNode::kChasing},
  // note: no loopback, as we want motion to be physics driven here
  END_OF_EDGES(PikminState)
};

Edge<PikminState> climbing_into_onion[] {
  // ClimbIntoOnion
  {kLastFrame, nullptr, EnterOnion, PikminNode::kClimbIntoOnion},
  // Note: while this is technically a loopback, the EnterOnion function
  // marks the pikmin as dead, removing it from the game. Thus, this state
  // runs to completion just once.
  END_OF_EDGES(PikminState)
};

Edge<PikminState> sliding_down_from_onion[] {
  // SlideDownFromOnion
  {kAlways, CollidedWithWhistle, WhistleOffOnion, PikminNode::kIdle},
  {kLastFrame, nullptr, HopOffFoot, PikminNode::kIdle},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> lift_treasure[] {
  {kAlways, CollidedWithWhistle, WhistleOffTreasure, PikminNode::kIdle},
  {kAlways, TreasureMoving, nullptr, PikminNode::kCarryTreasure},
  {kAlways, TreasureInvalid, RemoveFromTreasure, PikminNode::kIdle},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> carry_treasure[] {
  {kAlways, CollidedWithWhistle, WhistleOffTreasure, PikminNode::kIdle},
  {kAlways, TreasureStopped, nullptr, PikminNode::kLiftTreasure},
  {kAlways, TreasureInvalid, RemoveFromTreasure, PikminNode::kIdle},
  END_OF_EDGES(PikminState)
};

Node<PikminState> node_list[] {
  {"Init", true, init},
  {"Idle", true, idle, "Armature|Idle", 30},
  {"Grabbed", true, grabbed, "Armature|Idle", 30},
  {"Thrown", true, thrown, "Armature|Throw", 10},
  {"Targeting", true, targeting, "Armature|Run", 30},
  {"Chasing", true, chasing, "Armature|Run", 30},
  {"StandingAttack", true, standing_attack, "Armature|StandingAttack", 20},
  {"Jump", true, jumping, "Armature|Idle", 30},
  {"ClimbIntoOnion", true, climbing_into_onion, "Armature|Climb", 60},
  {"SlideDownFromOnion", true, sliding_down_from_onion, "Armature|Climb", 30},
  {"LiftTreasure", true, lift_treasure, "Armature|Lift", 123},
  {"CarryTreasure", true, carry_treasure, "Armature|Carry", 72},
};

StateMachine<PikminState> machine(node_list);

} // namespace pikmin_ai
