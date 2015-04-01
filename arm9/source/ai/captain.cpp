#include "captain.h"

#include <nds/arm9/input.h>
#include "dsgx.h"
#include "multipass_engine.h"
#include "game.h"

// Model data
#include "olimar_dsgx.h"
#include "olimar_low_poly_dsgx.h"

using numeric_types::literals::operator"" _f;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

using pikmin_ai::PikminState;
using pikmin_ai::PikminType;

namespace captain_ai {

Dsgx olimar_actor((u32*)olimar_dsgx, olimar_dsgx_size);
Dsgx olimar_low_poly_actor((u32*)olimar_low_poly_dsgx, olimar_low_poly_dsgx_size);

void InitAlways(CaptainState& captain) {
  //set the actor for animation
  captain.entity->set_actor(&olimar_low_poly_actor);

  //setup physics parameters for collision
  auto body = captain.entity->body();
  body->height = 6_f;
  body->radius = 1.5_f;

  body->collides_with_bodies = 1;

  //initialize our walking angle?
  captain.current_angle = 270_brad;
}

bool DpadActive(const CaptainState& captain) {
  return (keysHeld() & KEY_RIGHT) or 
         (keysHeld() & KEY_LEFT) or 
         (keysHeld() & KEY_UP) or 
         (keysHeld() & KEY_DOWN);
}

bool DpadInactive(const CaptainState& captain) {
  return !DpadActive(captain);
}

void StopCaptain(CaptainState& captain) {
  //reset velocity in XZ to 0, so we stop moving
  captain.entity->body()->velocity.x = 0_f;
  captain.entity->body()->velocity.z = 0_f;
}

void MoveCaptain(CaptainState& captain) {
  auto engine = captain.entity->engine();
  Brads dpad_angle = engine->CameraAngle() + engine->DPadDirection() - 90_brad;
  Brads delta = dpad_angle - captain.current_angle;

  // Translate delta to be in +/-180 degrees
  // Todo(Cristian) This may not be necessary anymore due to the use of Brads.
  while (delta >= 180_brad) {
    delta -= 360_brad;
  }
  while (delta < -180_brad) {
    delta += 360_brad;
  }

  //Now clamp delta, so that we achieve smooth turning angles
  if (delta > 11_brad) {
    delta = 11_brad;
  }
  if (delta < -11_brad) {
    delta = -11_brad;
  }

  captain.current_angle += delta;
  captain.entity->set_rotation(0_brad, captain.current_angle + 90_brad, 0_brad);

  // Apply velocity in the direction of the current angle.
  captain.entity->body()->velocity.x.data_ = cosLerp(captain.current_angle.data_);
  captain.entity->body()->velocity.z.data_ = -sinLerp(captain.current_angle.data_);
  captain.entity->body()->velocity.x *= 0.2_f;
  captain.entity->body()->velocity.z *= 0.2_f;
}

bool ActionDownNearPikmin(const CaptainState& captain) {
  if (keysDown() & KEY_A) {
    //TODO: Actually check for pikmin somehow. Don't do this noise.
    return true;
  }
  return false;
}

bool ActionReleased(const CaptainState& captain) {
  return (keysUp() & KEY_A);
}

void GrabPikmin(CaptainState& captain) {
  //Cheat horribly! Spawn a pikmin RIGHT NOW and hold onto it for dear life
  PikminState* pikmin = captain.game->SpawnObject<PikminState>();
  pikmin->type = PikminType::kBluePikmin;
  pikmin->entity->body()->position = captain.entity->body()->position;
  pikmin->parent = captain.entity;
  captain.held_pikmin = pikmin;
}

void ThrowPikmin(CaptainState& captain) {
  captain.held_pikmin->entity->body()->velocity = Vec3{0_f, 1_f, 0_f};
  captain.held_pikmin->parent = nullptr;
}

namespace CaptainNode {
enum CaptainNode {
  kInit = 0,
  kIdle,
  kRun,
  kGrab,
  kGrabRun,
  kThrow,
  kThrowRun,
};
}

Edge<CaptainState> edge_list[] {
  // Init
  Edge<CaptainState>{kAlways, nullptr, InitAlways, CaptainNode::kIdle},

  // Idle
  {kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrab},
  {kAlways, DpadActive, MoveCaptain, CaptainNode::kRun},

  // Run
  {kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrabRun},
  {kAlways, DpadInactive, StopCaptain, CaptainNode::kIdle},
  {kAlways, nullptr, MoveCaptain, CaptainNode::kRun},  // Loopback

  // Grab
  {kAlways, ActionReleased, ThrowPikmin, CaptainNode::kThrow},
  {kAlways, DpadActive, MoveCaptain, CaptainNode::kGrabRun},

  // GrabRun
  {kAlways, ActionReleased, ThrowPikmin, CaptainNode::kThrowRun},
  {kAlways, DpadInactive, StopCaptain, CaptainNode::kGrab},
  {kAlways, nullptr, MoveCaptain, CaptainNode::kGrabRun},  // Loopback

  // Throw
  {kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrab},
  {kAlways, DpadActive, MoveCaptain, CaptainNode::kThrowRun},
  {kLastFrame, nullptr, nullptr, CaptainNode::kIdle},

  // ThrowRun
  {kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrabRun},
  {kAlways, DpadInactive, StopCaptain, CaptainNode::kThrow},
  {kAlways, nullptr, MoveCaptain, CaptainNode::kThrowRun},  // Loopback
  {kLastFrame, nullptr, nullptr, CaptainNode::kRun},
};

Node node_list[] {
  {"Init", true, 0, 0},
  {"Idle", true, 1, 2, "Armature|Idle1", 30},
  {"Run", true, 3, 5, "Armature|Run", 60},
  {"Grab", true, 6, 7, "Armature|Idle1", 30},
  {"GrabRun", true, 8, 10, "Armature|Run", 60},
  {"Throw", true, 11, 13, "Armature|Idle1", 10},
  {"ThrowRun", true, 14, 17, "Armature|Run", 10},
};

StateMachine<CaptainState> machine(node_list, edge_list);

}