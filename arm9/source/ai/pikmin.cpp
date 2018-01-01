#include "pikmin.h"

#include "ai/health.h"
#include "ai/pikmin.h"
#include "ai/captain.h"
#include "ai/onion.h"
#include "ai/treasure.h"

#include "dsgx.h"
#include "pikmin_game.h"
#include "particle.h"
#include "particle_library.h"
#include "particle.h"
#include "particle_library.h"
#include "vector_utils.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;
using std::string;

using health_ai::HealthState;
using treasure_ai::TreasureState;

using physics::Body;

namespace pikmin_ai {

const fixed kRunSpeed = 40.0_f / 60_f;
const fixed kTargetThreshold = 2.0_f;

void SetPikminModel(PikminState& pikmin) {
  string color = "";
  // Set the initial mesh based on the pikmin's color and starting state
  switch (pikmin.type) {
    case PikminType::kNone:
    case PikminType::kRedPikmin:
      color = "red";
    break;
    case PikminType::kYellowPikmin:
      color = "yellow";
    break;
    case PikminType::kBluePikmin:
      color = "blue";
    break;
  }
  string mesh = color + "_pikmin";
  string actor = "pikmin";
  if (pikmin.current_node == PikminNode::kSeed) {
    mesh = color + "_seed";
    actor = "pikmin_seed";
  }

  pikmin.entity->set_actor(pikmin.game->ActorAllocator()->Retrieve(actor.c_str()));
  pikmin.entity->set_mesh(mesh.c_str());
}

void InitAlways(PikminState& pikmin) {
  pikmin.body->height = 6_f;
  pikmin.body->radius = 1.0_f;

  pikmin.body->collides_with_bodies = 1;
  pikmin.body->is_pikmin = 1;
  pikmin.body->is_movable = 1;
  pikmin.body->sensor_groups = WHISTLE_GROUP | DETECT_GROUP | ATTACK_GROUP | TREASURE_GROUP;

  pikmin.entity->important = false;

  pikmin.current_node = pikmin.starting_state;
  SetPikminModel(pikmin);
}

void IdleAlways(PikminState& pikmin) {
  if (pikmin.current_squad) {
    //every 8 frames or so, update our facing direction to look at the captain
    if ((pikmin.game->CurrentFrame() + pikmin.handle.id) % 8 == 0) {
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
  return (pikmin.game->CurrentFrame() % 100) == (pikmin.handle.id % 100);
}

bool HasNewParent(const PikminState& pikmin) {
  return pikmin.parent != nullptr;
}

void StoreParentLocation(PikminState& pikmin) {
  if (pikmin.parent) {
    pikmin.parent_initial_location = pikmin.parent->position();
    pikmin.child_offset = pikmin.parent_initial_location
        - pikmin.position();
  }
  pikmin.set_velocity(Vec3{0_f,0_f,0_f});
}

void FollowParent(PikminState& pikmin) {
  if (pikmin.parent) {
    pikmin.set_position(pikmin.parent->position()
        + pikmin.child_offset);
  }
}

bool LeftParent(const PikminState& pikmin) {
  return pikmin.parent == nullptr;
}

void StopMoving(PikminState& pikmin) {
  pikmin.set_velocity(Vec3{0_f, 0_f, 0_f});
}

void ClearTargetAndStop(PikminState& pikmin) {
  pikmin.has_target = false;
  StopMoving(pikmin);
}

bool Landed(const PikminState& pikmin) {
  return pikmin.body->touching_ground;
}

void FaceTarget(PikminState& pikmin) {
  Vec2 posXZ{pikmin.body->position.x, pikmin.body->position.z};
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
  pikmin.body->velocity.x = new_velocity.x;
  pikmin.body->velocity.z = new_velocity.y;
  pikmin.entity->set_rotation(0_brad, AngleFromNormalizedVec2(new_direction), 0_brad);
}

void RunToTarget(PikminState& pikmin) {
  // Only update the angle every so often, as this is expensive!
  if (((pikmin.handle.id + pikmin.game->CurrentFrame()) & 0x3) == 0) {
    FaceTarget(pikmin);
  }
}

bool PikminTurn(const PikminState& pikmin) {
  return pikmin.game->CurrentFrame() % 100 == pikmin.handle.id;
}

template <int Chance>
bool RandomTurnChance(const PikminState& pikmin) {
  return PikminTurn(pikmin) and rand() % 100 < Chance;
}

void ChooseRandomTarget(PikminState& pikmin) {
  Vec2 new_target{pikmin.position().x, pikmin.position().z};
  new_target.x += fixed::FromInt((rand() % 30) - 15);
  new_target.y += fixed::FromInt((rand() % 30) - 15);
  pikmin.target = new_target;
}

bool TargetReached(const PikminState& pikmin) {
  //don't do this every frame, for intentional inaccuracy
  if ((pikmin.handle.id + pikmin.game->CurrentFrame()) % 16 == 0) {
    auto position = pikmin.position();
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
  auto position = pikmin.position();
  return (pikmin.target - Vec2{position.x, position.z}).Length2() >
      kTargetThreshold * kTargetThreshold;
}

bool CollidedWithWhistle(const PikminState& pikmin) {
  if (pikmin.current_squad == nullptr) {
    if (pikmin.body->result_groups & WHISTLE_GROUP) {
      return true;
    }
  }
  return false;
}

void JoinSquad(PikminState& pikmin) {
  auto result = pikmin.body->FirstCollisionWith(WHISTLE_GROUP);
  // make sure we got a real result (this can fail in extreme cases)
  if (result.body) {

    auto captain = pikmin.game->RetrieveCaptain(result.body->owner);
    if (captain) {
      pikmin.current_squad = &captain->squad;
      captain->squad.AddPikmin(&pikmin);
    }
  }
}

bool ChaseTargetInvalid(const PikminState& pikmin) {
  // Some unspeakable horror caused our target to vanish or otherwise change
  if (pikmin.world().RetrieveBody(pikmin.chase_target_body) == nullptr) {
    return true;
  }
  return false;
}

bool CollideWithAttackable(const PikminState& pikmin) {
  if (pikmin.body->result_groups & ATTACK_GROUP) {
    return true;
  }
  return false;
}

void SetAttackTarget(PikminState& pikmin) {
  auto result = pikmin.body->FirstCollisionWith(ATTACK_GROUP);
  if (result.body) {
    pikmin.attack_target_body = result.body->handle;
  }
  StopMoving(pikmin);
}

void ChaseTarget(PikminState& pikmin) {
  if (Body* chase_target = pikmin.world().RetrieveBody(pikmin.chase_target_body)) {
    pikmin.target = Vec2{chase_target->position.x, chase_target->position.z};
    RunToTarget(pikmin);
  }
}

void DealDamageToTarget(PikminState& pikmin) {
  if (Body* chase_target = pikmin.world().RetrieveBody(pikmin.attack_target_body)) {
    if (HealthState* enemy_health = pikmin.game->RetrieveHealth(chase_target->owner)) {
      enemy_health->DealDamage(5);
    }
  }
}

void JumpTowardTarget(PikminState& pikmin) {
  // Face the target, then apply upwards velocity
  FaceTarget(pikmin);
  pikmin.body->velocity.y = 0.4_f;
}

bool CollideWithTarget(const PikminState& pikmin) {
  if (pikmin.current_squad) {
    return false;
  }
  if (pikmin.body->result_groups & DETECT_GROUP) {
    auto target_circle = pikmin.body->FirstCollisionWith(DETECT_GROUP);
    if (target_circle.body) {
      return true;
    }
  }
  return false;
}

void StoreTargetBody(PikminState& pikmin) {
  auto target_circle = pikmin.body->FirstCollisionWith(DETECT_GROUP);
  if (target_circle.body) {
    pikmin.chase_target_body = target_circle.body->handle;
  }
}

void Aim(PikminState& pikmin) {
  FaceTarget(pikmin);
  pikmin.set_velocity(Vec3{0_f,0_f,0_f});
}

bool CollideWithOnionFoot(const PikminState& pikmin) {
  if (pikmin.body->result_groups & ONION_FEET_GROUP) {
    return true;
  }
  return false;
}

void StartClimbingOnion(PikminState& pikmin) {
  // Grab the onion / foot that we're targeting
  auto onion_foot = pikmin.body->FirstCollisionWith(ONION_FEET_GROUP);
  fixed travel_frames = 60_f;
  if (onion_foot.body) {
    auto onion = pikmin.game->RetrieveOnion(onion_foot.body->owner);
    auto pikmin_body = pikmin.body;
    pikmin_body->position = onion_foot.body->position;
    pikmin_body->affected_by_gravity = false;
    pikmin_body->collision_group = 0;
    pikmin_body->sensor_groups = 0;
    pikmin_body->is_movable = 0;

    pikmin.entity->RotateToFace(onion->entity);
    auto onion_position = onion->position();
    Vec2 climb_xz = (
        Vec2{onion_position.x, onion_position.z} -
        Vec2{pikmin_body->position.x, pikmin_body->position.z}
      ).Normalize();
    climb_xz = climb_xz * 9.2_f;
    pikmin.set_velocity(Vec3{
      climb_xz.x / travel_frames,
      9.1_f / travel_frames,
      climb_xz.y / travel_frames
    });
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
  pikmin.body->affected_by_gravity = true;
  pikmin.game->RetrieveCaptain(pikmin.game->ActiveCaptain())->squad.AddPikmin(&pikmin);
  pikmin.body->velocity.y = 0.5_f;
}

void HopOffFoot(PikminState& pikmin) {
  pikmin.body->affected_by_gravity = true;
  pikmin.body->velocity.y = 1.0_f;

  pikmin.game->RetrieveCaptain(pikmin.game->ActiveCaptain())->squad.AddPikmin(&pikmin);
}

bool CollideWithValidTreasure(const PikminState& pikmin) {
  if (pikmin.body->result_groups & TREASURE_GROUP) {
    auto treasure_result = pikmin.body->FirstCollisionWith(TREASURE_GROUP);
    if (treasure_result.body != nullptr) {
      auto treasure = pikmin.game->RetrieveTreasure(treasure_result.body->owner);
      if (treasure->RoomForMorePikmin()) {
        return true;
      }
    }
  }
  return false;
}

void AddToTreasure(PikminState& pikmin) {
  auto treasure_result = pikmin.body->FirstCollisionWith(TREASURE_GROUP);
  if (treasure_result.body != nullptr) {
    pikmin.active_treasure = treasure_result.body->owner;
    TreasureState* treasure = pikmin.game->RetrieveTreasure(pikmin.active_treasure);
    if (treasure) {
      treasure->AddPikmin(&pikmin);
    }
  }
  StopMoving(pikmin);
  pikmin.body->affected_by_gravity = false;
}

void RemoveFromTreasure(PikminState& pikmin) {
  auto treasure = pikmin.game->RetrieveTreasure(pikmin.active_treasure);
  if (treasure) {
    treasure->RemovePikmin(&pikmin);
    pikmin.active_treasure = Handle();
  }
  pikmin.body->affected_by_gravity = true;
}

void WhistleOffTreasure(PikminState& pikmin) {
  RemoveFromTreasure(pikmin);
  JoinSquad(pikmin);
}

bool TreasureMoving(const PikminState& pikmin) {
  auto treasure = pikmin.game->RetrieveTreasure(pikmin.active_treasure);
  return treasure and treasure->Moving();
}

bool TreasureInvalid(const PikminState& pikmin) {
  auto treasure = pikmin.game->RetrieveTreasure(pikmin.active_treasure);
  if (treasure == nullptr) {
    return true;
  }
  if (treasure->carryable == false) {
    return true;
  }
  return false;
}

bool TreasureStopped(const PikminState& pikmin) {
  return !TreasureMoving(pikmin);
}

void IssueThrowParticles(PikminState& pikmin) {
  if ((pikmin.frames_at_this_node & 0x3) == 0) {
    Particle* star = SpawnParticle(particle_library::piki_star);
    star->position = pikmin.position();
    particle_library::SpreadPikiStar(star);
  }
}

void FloatGently(PikminState& pikmin) {
  pikmin.body->acceleration.y = GRAVITY_CONSTANT * 0.82_f;
  auto rotation = pikmin.entity->rotation();
  rotation.z = 140_brad - ((140_brad / 40) * pikmin.frames_at_this_node); // vary!
  if (rotation.z < 0_brad) {
    rotation.z = 0_brad;
  }
  pikmin.entity->set_rotation(rotation);
  pikmin.body->radius = 0_f;
  if (pikmin.velocity().y < -0.4_f) {
    pikmin.body->velocity.y = -0.4_f;
  }

  IssueThrowParticles(pikmin);
}

void CreateDirtCloud(PikminState& pikmin) {
  // Spawn some flying dirt, for science
  for (int i = 0; i < 2; i++) {
    Particle* rock_particle = SpawnParticle(particle_library::rock);
    rock_particle->position = pikmin.position();
    rock_particle->velocity += particle_library::RockSpread();
  }
  for (int j = 0; j < 6; j++) {
    Particle* dirt_cloud = SpawnParticle(particle_library::dirt_cloud);
    dirt_cloud->position = pikmin.position();
    dirt_cloud->position.y += 0.75_f;
    dirt_cloud->velocity += particle_library::DirtCloudSpread();
  }
}

void PlantSeed(PikminState& pikmin) {
  pikmin.body->acceleration.y = 0_f;
  pikmin.set_velocity(Vec3{0_f,0_f,0_f});
  SetPikminModel(pikmin);

  CreateDirtCloud(pikmin);
}

bool PikminPlucked(const PikminState& pikmin) {
  return false;
}

void ResetPhysics(PikminState& pikmin) {
  // What did I mean to do here?
}

void PluckIntoSquad(PikminState& pikmin) {
  pikmin.body->radius = 1_f;
  JoinSquad(pikmin);
  CreateDirtCloud(pikmin);
}

Edge<PikminState> init[] {
  // Init
  {InitAlways, PikminNode::kIdle},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> idle[] {
  // Idle
  {CollideWithOnionFoot, StartClimbingOnion, PikminNode::kClimbIntoOnion},
  {TooFarFromTarget, PikminNode::kTargeting},
  {CollidedWithWhistle, JoinSquad, PikminNode::kIdle},
  {HasNewParent, StoreParentLocation, PikminNode::kGrabbed},
  {CollideWithTarget, StoreTargetBody, PikminNode::kChasing},
  {IdleAlways, PikminNode::kIdle}, // Loopback
  END_OF_EDGES(PikminState)
};

Edge<PikminState> grabbed[] {
  // Grabbed
  {LeftParent, PikminNode::kThrown},
  {FollowParent, PikminNode::kGrabbed},  // Loopback
  END_OF_EDGES(PikminState)
};

Edge<PikminState> thrown[] {
  // Thrown
  {Landed, StopMoving, PikminNode::kIdle},
  {IssueThrowParticles, PikminNode::kThrown},  // Loopback
  END_OF_EDGES(PikminState)
};

Edge<PikminState> targeting[] {
  // Targeting
  {CollideWithOnionFoot, StartClimbingOnion, PikminNode::kClimbIntoOnion},
  {TargetReached, ClearTargetAndStop, PikminNode::kIdle},
  {CantReachTarget, ClearTargetAndStop, PikminNode::kIdle},
  {HasNewParent, StoreParentLocation, PikminNode::kGrabbed},
  {RunToTarget, PikminNode::kTargeting},  // loopback
  END_OF_EDGES(PikminState)
};

Edge<PikminState> chasing[] {
  // Chasing (Attack, Work, Carry)
  {CantReachTarget, ClearTargetAndStop, PikminNode::kIdle},
  {ChaseTargetInvalid, ClearTargetAndStop, PikminNode::kIdle},
  {CollidedWithWhistle, JoinSquad, PikminNode::kIdle},
  {CollideWithAttackable, SetAttackTarget, PikminNode::kStandingAttack},
  {CollideWithValidTreasure, AddToTreasure, PikminNode::kLiftTreasure},
  {ChaseTarget, PikminNode::kChasing},  // loopback
  END_OF_EDGES(PikminState)
};

Edge<PikminState> standing_attack[] {
  // Standing Attack
  {ChaseTargetInvalid, ClearTargetAndStop, PikminNode::kIdle},
  {Trigger::kFirstFrame, Aim, PikminNode::kStandingAttack},
  {Trigger::kLastFrame, DealDamageToTarget, PikminNode::kJump},
  {CollidedWithWhistle, JoinSquad, PikminNode::kIdle},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> jumping[] {
  // Jump
  {ChaseTargetInvalid, ClearTargetAndStop, PikminNode::kIdle},
  {Trigger::kFirstFrame, JumpTowardTarget, PikminNode::kJump},
  {CollidedWithWhistle, JoinSquad, PikminNode::kIdle},
  {Landed, StopMoving, PikminNode::kChasing},
  // note: no loopback, as we want motion to be physics driven here
  END_OF_EDGES(PikminState)
};

Edge<PikminState> climbing_into_onion[] {
  // ClimbIntoOnion
  {Trigger::kLastFrame, EnterOnion, PikminNode::kClimbIntoOnion},
  // Note: while this is technically a loopback, the EnterOnion function
  // marks the pikmin as dead, removing it from the game. Thus, this state
  // runs to completion just once.
  END_OF_EDGES(PikminState)
};

Edge<PikminState> sliding_down_from_onion[] {
  // SlideDownFromOnion
  {CollidedWithWhistle, WhistleOffOnion, PikminNode::kIdle},
  {Trigger::kLastFrame, HopOffFoot, PikminNode::kIdle},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> lift_treasure[] {
  {CollidedWithWhistle, WhistleOffTreasure, PikminNode::kIdle},
  {TreasureMoving, PikminNode::kCarryTreasure},
  {TreasureInvalid, RemoveFromTreasure, PikminNode::kIdle},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> carry_treasure[] {
  {CollidedWithWhistle, WhistleOffTreasure, PikminNode::kIdle},
  {TreasureStopped, PikminNode::kLiftTreasure},
  {TreasureInvalid, RemoveFromTreasure, PikminNode::kIdle},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> seed[] {
  {Landed, PlantSeed, PikminNode::kGrowing},
  {FloatGently, PikminNode::kSeed},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> growing[] {
  {Trigger::kLastFrame, PikminNode::kSprout},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> sprout[] {
  {PikminPlucked, PikminNode::kPlucked},
  {CollidedWithWhistle, PluckIntoSquad, PikminNode::kPlucked},
  END_OF_EDGES(PikminState)
};

Edge<PikminState> plucked[] {
  {Trigger::kLastFrame, PikminNode::kIdle},
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
  {"Seed", true, seed, "Armature|twirl_about", 24},
  {"Growing", true, growing, "Armature|Grow", 56},
  {"Sprout", true, sprout, "Armature|Planted", 40},
  {"Plucked", true, plucked, "Armature|Plucked", 25},
};

StateMachine<PikminState> machine(node_list);

} // namespace pikmin_ai
