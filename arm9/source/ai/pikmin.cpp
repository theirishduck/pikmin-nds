#include "pikmin.h"

#include "dsgx.h"
#include "multipass_engine.h"
#include "red_pikmin_dsgx.h"
#include "yellow_pikmin_dsgx.h"
#include "blue_pikmin_dsgx.h"

using numeric_types::literals::operator"" _f;

namespace pikmin_ai {

Dsgx red_pikmin_actor((u32*)red_pikmin_dsgx, red_pikmin_dsgx_size);
Dsgx yellow_pikmin_actor((u32*)yellow_pikmin_dsgx, yellow_pikmin_dsgx_size);
Dsgx blue_pikmin_actor((u32*)blue_pikmin_dsgx, blue_pikmin_dsgx_size);

void InitAlways(PikminState& pikmin) {
  switch (pikmin.type) {
    case PikminType::kRedPikmin:
      pikmin.entity->set_actor(&red_pikmin_actor);
      break;
    case PikminType::kYellowPikmin:
      pikmin.entity->set_actor(&yellow_pikmin_actor);
      break;
    case PikminType::kBluePikmin:
      pikmin.entity->set_actor(&blue_pikmin_actor);
      break;
  }
}

void IdleAlways(PikminState& pikmin) {

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

bool Landed(const PikminState& pikmin) {
  return pikmin.entity->body()->touching_ground;
}



namespace PikminNode {
enum PikminNode {
  kInit = 0,
  kIdle,
  kGrabbed,
  kThrown,
};
}

Edge<PikminState> edge_list[] {
  //Init
  Edge<PikminState>{kAlways, nullptr, InitAlways, PikminNode::kIdle},

  //Idle
  {kAlways, HasNewParent, StoreParentLocation, PikminNode::kGrabbed},
  {kAlways,nullptr,IdleAlways,PikminNode::kIdle}, // Loopback

  //Grabbed
  {kAlways, LeftParent, nullptr, PikminNode::kThrown},
  {kAlways, nullptr, FollowParent, PikminNode::kGrabbed},  // Loopback

  //Thrown
  {kAlways, Landed, StopMoving, PikminNode::kIdle},

};

Node node_list[] {
  {"Init", true, 0, 0},
  {"Idle", true, 1, 2, "Armature|Idle", 60},
  {"Grabbed", true, 3, 4, "Armature|Idle", 60},
  {"Thrown", true, 5, 5, "Armature|Throw", 20},
};

StateMachine<PikminState> machine(node_list, edge_list);

} // namespace pikmin_ai
