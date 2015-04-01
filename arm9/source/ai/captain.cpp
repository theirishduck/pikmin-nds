#include "captain.h"

#include <nds/arm9/input.h>
#include "dsgx.h"
#include "multipass_engine.h"
#include "game.h"

// Model data
#include "olimar_dsgx.h"
#include "olimar_low_poly_dsgx.h"
#include "cursor_dsgx.h"

using numeric_types::literals::operator"" _f;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

using pikmin_ai::PikminState;
using pikmin_ai::PikminType;

namespace captain_ai {

Dsgx olimar_actor((u32*)olimar_dsgx, olimar_dsgx_size);
Dsgx olimar_low_poly_actor((u32*)olimar_low_poly_dsgx, olimar_low_poly_dsgx_size);
Dsgx cursor_actor((u32*)cursor_dsgx, cursor_dsgx_size);

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

  //Initialize the cursor
  captain.cursor->set_actor(&cursor_actor);
  cursor_actor.ApplyTextures(captain.game->TextureAllocator());
  captain.cursor->body()->ignores_walls = 1;
  captain.cursor->body()->position = body->position
      + Vec3{0_f,0_f,5_f};
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
  captain.cursor->body()->velocity.x = 0_f;
  captain.cursor->body()->velocity.z = 0_f;
   
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

  captain.current_angle = dpad_angle;
  captain.entity->set_rotation(0_brad, captain.current_angle + 90_brad, 0_brad);

  // Apply velocity in the direction of the current angle.
  captain.entity->body()->velocity.x.data_ = cosLerp(captain.current_angle.data_);
  captain.entity->body()->velocity.z.data_ = -sinLerp(captain.current_angle.data_);
  captain.entity->body()->velocity.x *= 0.2_f;
  captain.entity->body()->velocity.z *= 0.2_f;

  captain.cursor->body()->velocity.x = captain.entity->body()->velocity.x * 3_f;
  captain.cursor->body()->velocity.z = captain.entity->body()->velocity.z * 3_f;

  // Clamp the cursor to a certain distance from the captain
  Vec2 captain_xz = Vec2{captain.entity->body()->position.x, captain.entity->body()->position.z};
  Vec2 cursor_xz = Vec2{captain.cursor->body()->position.x, captain.cursor->body()->position.z};
  fixed distance = (cursor_xz - captain_xz).Length();
  if (distance > 15_f) {
    cursor_xz = (cursor_xz - captain_xz).Normalize() * 15_f;
    cursor_xz += captain_xz;
    captain.cursor->body()->position.x = cursor_xz.x;
    captain.cursor->body()->position.z = cursor_xz.y;
  }
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

  //Move the pikmin to olimar's hand
  pikmin->entity->body()->position = captain.entity->body()->position;
  pikmin->entity->body()->position.x.data_ += cosLerp(captain.current_angle.data_);
  pikmin->entity->body()->position.y += 0.5_f;
  pikmin->entity->body()->position.z.data_ += -sinLerp(captain.current_angle.data_);
  pikmin->parent = captain.entity;
  captain.held_pikmin = pikmin;
}

void ThrowPikmin(CaptainState& captain) {
  fixed pikmin_y_velocity = 1_f;
  if (captain.held_pikmin->type == PikminType::kYellowPikmin) {
    pikmin_y_velocity = 1.2_f;
  }

  fixed pikmin_travel_time = pikmin_y_velocity * 2_f / GRAVITY_CONSTANT;
  Vec3 distance_to_cursor = captain.cursor->body()->position - 
      captain.entity->body()->position;

  fixed pikmin_x_velocity = distance_to_cursor.x / pikmin_travel_time;
  fixed pikmin_z_velocity = distance_to_cursor.z / pikmin_travel_time;

  captain.held_pikmin->entity->body()->velocity = Vec3{
      pikmin_x_velocity, pikmin_y_velocity, pikmin_z_velocity};
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
