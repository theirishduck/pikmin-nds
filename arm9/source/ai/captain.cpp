#include "captain.h"

#include <nds/arm9/input.h>
#include "dsgx.h"
#include "multipass_engine.h"
#include "game.h"

// Model data
#include "olimar_dsgx.h"
#include "olimar_low_poly_dsgx.h"
#include "cursor_dsgx.h"
#include "whistle_dsgx.h"

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
Dsgx whistle_actor((u32*)whistle_dsgx, whistle_dsgx_size);

void HandleWhistle(CaptainState& captain) {
  captain.whistle->body()->position = captain.cursor->body()->position;

  // Do a bit of cheating and handle the whistle here for now
  if (keysHeld() & KEY_B) {
    if (captain.whistle_timer < 16) {
      captain.whistle_timer++;
      //turn the whistle on
      captain.whistle->body()->collision_group = WHISTLE_GROUP;
    }
  } else if (captain.whistle_timer > 0) {
    captain.whistle_timer--;
    //turn the whistle back off
    captain.whistle->body()->collision_group = 0;
  }

  captain.whistle->body()->radius = fixed::FromInt(captain.whistle_timer) * 10_f / 16_f;
  captain.whistle->set_scale(fixed::FromInt(captain.whistle_timer) * 10_f / 16_f);
  captain.whistle->set_rotation(0_brad, captain.whistle->rotation().y + 3_brad, 0_brad);
}

void InitAlways(CaptainState& captain) {
  //set the actor for animation
  captain.entity->set_actor(&olimar_low_poly_actor);

  //setup physics parameters for collision
  auto body = captain.entity->body();
  body->height = 6_f;
  body->radius = 1.5_f;

  body->collides_with_bodies = 1;
  body->collision_group = PLAYER_GROUP | WHISTLE_GROUP;
  body->owner = &captain;

  //initialize our walking angle?
  captain.current_angle = 270_brad;

  //Initialize the cursor
  captain.cursor->set_actor(&cursor_actor);
  cursor_actor.ApplyTextures(captain.game->TextureAllocator());
  captain.cursor->body()->ignores_walls = 1;
  captain.cursor->body()->position = body->position
      + Vec3{0_f,0_f,5_f};
  captain.cursor->body()->is_sensor = 1;

  //Initialize the whistle
  captain.whistle->set_actor(&whistle_actor);
  whistle_actor.ApplyTextures(captain.game->TextureAllocator());
  auto whistle_body = captain.whistle->body();

  whistle_body->height = 10.0_f;
  whistle_body->is_sensor = 1;
  whistle_body->owner = &captain;
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
  auto body = captain.entity->body();
  body->velocity.x = 0_f;
  body->velocity.z = 0_f;
  auto cursor_body = captain.cursor->body();
  cursor_body->velocity.x = 0_f;
  cursor_body->velocity.z = 0_f;
}

void IdleAlways(CaptainState& captain) {
  HandleWhistle(captain);
}

void MoveCaptain(CaptainState& captain) {
  auto engine = captain.entity->engine();
  Brads dpad_angle = engine->CameraAngle() + engine->DPadDirection() - 90_brad;
  captain.current_angle = dpad_angle;
  captain.entity->RotateToFace(captain.current_angle + 90_brad, 10_brad);

  // Apply velocity in the direction of the current angle.
  auto body = captain.entity->body();
  body->velocity.x.data_ = cosLerp(captain.current_angle.data_);
  body->velocity.z.data_ = -sinLerp(captain.current_angle.data_);
  body->velocity.x *= 0.4_f;
  body->velocity.z *= 0.4_f;

  auto cursor_body = captain.cursor->body();
  cursor_body->velocity.x = body->velocity.x * 4_f;
  cursor_body->velocity.z = body->velocity.z * 4_f;

  // Clamp the cursor to a certain distance from the captain
  Vec2 captain_xz = Vec2{body->position.x, body->position.z};
  Vec2 cursor_xz = Vec2{cursor_body->position.x, cursor_body->position.z};
  fixed distance = (cursor_xz - captain_xz).Length();
  if (distance > 14_f) {
    cursor_xz = (cursor_xz - captain_xz).Normalize() * 14_f;
    cursor_xz += captain_xz;
    cursor_body->position.x = cursor_xz.x;
    cursor_body->position.z = cursor_xz.y;
  }

  // Rotate the cursor so that it faces away from the captain
  captain.cursor->RotateToFace(captain.entity->AngleTo(captain.cursor) - 90_brad);

  // Move the whistle to where the cursor is
  HandleWhistle(captain);
}

bool ActionDownNearPikmin(const CaptainState& captain) {
  if (keysDown() & KEY_A and captain.squad.squad_size > 0) {
    //todo: check for proximity? this will work for now I guess
    return true;
  }
  return false;
}

bool ActionReleased(const CaptainState& captain) {
  return (keysUp() & KEY_A);
}

void GrabPikmin(CaptainState& captain) {
  //grab the first pikmin in the squad
  //TODO: this should later be changed to grab the *closest pikmin*
  PikminState* pikmin = captain.squad.pikmin[0];
  captain.squad.RemovePikmin(pikmin);

  //Move the pikmin to olimar's hand
  auto pikmin_body = pikmin->entity->body();
  pikmin_body->position = captain.entity->body()->position;
  pikmin_body->position.x.data_ += cosLerp(captain.current_angle.data_);
  pikmin_body->position.y += 0.5_f;
  pikmin_body->position.z.data_ += -sinLerp(captain.current_angle.data_);
  pikmin->parent = captain.entity;
  captain.held_pikmin = pikmin;
}

void ThrowPikmin(CaptainState& captain) {
  fixed pikmin_y_velocity = 0.8_f;
  if (captain.held_pikmin->type == PikminType::kYellowPikmin) {
    pikmin_y_velocity = 1.0_f;
  }

  fixed pikmin_travel_time = pikmin_y_velocity * 2_f / GRAVITY_CONSTANT;
  Vec3 distance_to_cursor = captain.cursor->body()->position - 
      captain.entity->body()->position;

  fixed pikmin_x_velocity = distance_to_cursor.x / pikmin_travel_time;
  fixed pikmin_z_velocity = distance_to_cursor.z / pikmin_travel_time;

  // Add in the captain's velocity; this is an intended gameplay mechanic
  pikmin_x_velocity += captain.entity->body()->velocity.x;
  pikmin_z_velocity += captain.entity->body()->velocity.z;

  captain.held_pikmin->entity->body()->velocity = Vec3{
      pikmin_x_velocity, pikmin_y_velocity, pikmin_z_velocity};
  captain.held_pikmin->parent = nullptr;

  captain.held_pikmin->entity->RotateToFace(captain.cursor);
  captain.held_pikmin = nullptr;
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
  {kAlways, nullptr, IdleAlways, CaptainNode::kIdle},  // Loopback

  // Run
  {kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrabRun},
  {kAlways, DpadInactive, StopCaptain, CaptainNode::kIdle},
  {kAlways, nullptr, MoveCaptain, CaptainNode::kRun},  // Loopback

  // Grab
  {kAlways, ActionReleased, ThrowPikmin, CaptainNode::kThrow},
  {kAlways, DpadActive, MoveCaptain, CaptainNode::kGrabRun},
  {kAlways, nullptr, IdleAlways, CaptainNode::kGrab},  // Loopback

  // GrabRun
  {kAlways, ActionReleased, ThrowPikmin, CaptainNode::kThrowRun},
  {kAlways, DpadInactive, StopCaptain, CaptainNode::kGrab},
  {kAlways, nullptr, MoveCaptain, CaptainNode::kGrabRun},  // Loopback

  // Throw
  {kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrab},
  {kAlways, DpadActive, MoveCaptain, CaptainNode::kThrowRun},
  {kLastFrame, nullptr, IdleAlways, CaptainNode::kIdle},
  {kAlways, nullptr, IdleAlways, CaptainNode::kThrow},  // Loopback

  // ThrowRun
  {kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrabRun},
  {kAlways, DpadInactive, StopCaptain, CaptainNode::kThrow},
  {kAlways, nullptr, MoveCaptain, CaptainNode::kThrowRun},  // Loopback
  {kLastFrame, nullptr, nullptr, CaptainNode::kRun},
};

Node node_list[] {
  {"Init", true, 0, 0},
  {"Idle", true, 1, 3, "Armature|Idle1", 30},
  {"Run", true, 4, 6, "Armature|Run", 60},
  {"Grab", true, 7, 9, "Armature|Idle1", 30},
  {"GrabRun", true, 10, 12, "Armature|Run", 60},
  {"Throw", true, 13, 16, "Armature|Idle1", 10},
  {"ThrowRun", true, 17, 20, "Armature|Run", 10},
};

StateMachine<CaptainState> machine(node_list, edge_list);

}
