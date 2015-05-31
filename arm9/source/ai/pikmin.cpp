#include "pikmin.h"
#include "captain.h"
#include "pikmin_game.h"

#include "dsgx.h"
#include "multipass_engine.h"
#include "red_pikmin_dsgx.h"
#include "yellow_pikmin_dsgx.h"
#include "blue_pikmin_dsgx.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

namespace pikmin_ai {

Dsgx red_pikmin_actor((u32*)red_pikmin_dsgx, red_pikmin_dsgx_size);
Dsgx yellow_pikmin_actor((u32*)yellow_pikmin_dsgx, yellow_pikmin_dsgx_size);
Dsgx blue_pikmin_actor((u32*)blue_pikmin_dsgx, blue_pikmin_dsgx_size);

void InitAlways(PikminState& pikmin) {
  switch (pikmin.type) {
    case PikminType::kNone:
    case PikminType::kRedPikmin:
      pikmin.entity->set_actor(&red_pikmin_actor);
      red_pikmin_actor.ApplyTextures(pikmin.game->TextureAllocator(), pikmin.game->TexturePaletteAllocator());
      break;
    case PikminType::kYellowPikmin:
      pikmin.entity->set_actor(&yellow_pikmin_actor);
      yellow_pikmin_actor.ApplyTextures(pikmin.game->TextureAllocator(), pikmin.game->TexturePaletteAllocator());
      break;
    case PikminType::kBluePikmin:
      pikmin.entity->set_actor(&blue_pikmin_actor);
      blue_pikmin_actor.ApplyTextures(pikmin.game->TextureAllocator(), pikmin.game->TexturePaletteAllocator());
      break;
  }

  auto body = pikmin.entity->body();
  body->height = 6_f;
  body->radius = 1.5_f;

  body->collides_with_bodies = 1;
  body->is_pikmin = 1;
  body->is_movable = 1;
  body->sensor_groups = WHISTLE_GROUP;

  pikmin.entity->important = false;
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

const fixed kRunningSpeed = 20.0_f / 60_f;

void FaceTarget(PikminState& pikmin) {
  if ((pikmin.id + pikmin.entity->engine()->FrameCounter()) % 4 == 0) {
    auto body = pikmin.entity->body();
    Vec2 posXZ{body->position.x, body->position.z};
    Vec2 random_offset = Vec2{
      fixed::FromInt(rand() % 10) / 5_f - 0.5_f,
      fixed::FromInt(rand() % 10) / 5_f - 0.5_f,
    };
    Vec2 new_velocity = (pikmin.target + random_offset - posXZ).Normalize() * kRunningSpeed;
    body->velocity.x = new_velocity.x;
    body->velocity.z = new_velocity.y;
    pikmin.entity->RotateToXZDirection(new_velocity);
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

const fixed kTargetThreshold = 2.0_f;

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


namespace PikminNode {
enum PikminNode {
  kInit = 0,
  kIdle,
  kGrabbed,
  kThrown,
  kTargeting,
};
}

Edge<PikminState> edge_list[] {
  //Init
  Edge<PikminState>{kAlways, nullptr, InitAlways, PikminNode::kIdle},

  //Idle
  {kAlways, TooFarFromTarget, nullptr, PikminNode::kTargeting},
  {kAlways, CollidedWithWhistle, JoinSquad, PikminNode::kIdle},
  {kAlways, HasNewParent, StoreParentLocation, PikminNode::kGrabbed},
  {kAlways,nullptr,IdleAlways,PikminNode::kIdle}, // Loopback

  //Grabbed
  {kAlways, LeftParent, nullptr, PikminNode::kThrown},
  {kAlways, nullptr, FollowParent, PikminNode::kGrabbed},  // Loopback

  //Thrown
  {kAlways, Landed, StopMoving, PikminNode::kIdle},

  //Targeting
  {kAlways, TargetReached, ClearTargetAndStop, PikminNode::kIdle},
  {kAlways, CantReachTarget, StopMoving, PikminNode::kIdle},
  {kAlways, HasNewParent, StoreParentLocation, PikminNode::kGrabbed},
  {kAlways, nullptr, FaceTarget, PikminNode::kTargeting},

};

Node node_list[] {
  {"Init", true, 0, 0},
  {"Idle", true, 1, 4, "Armature|Idle", 60},
  {"Grabbed", true, 5, 6, "Armature|Idle", 60},
  {"Thrown", true, 7, 7, "Armature|Throw", 20},
  {"Targeting", true, 8, 11, "Armature|Run", 60},

};

StateMachine<PikminState> machine(node_list, edge_list);

} // namespace pikmin_ai
