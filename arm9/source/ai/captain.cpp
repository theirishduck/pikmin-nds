#include "captain.h"

#include <nds/arm9/input.h>
#include "dsgx.h"
#include "multipass_engine.h"
#include "pikmin_game.h"
#include "trig.h"
#include "ai/onion.h"

// Model data
//#include "olimar_dsgx.h"
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

// Useful Constants
const fixed kRunSpeed = 0.8_f;
const fixed kCursorMaxDistance = 14_f;
const fixed kCursorSpeedMultiplier = 3_f;
const fixed kPikminThrowHeight = 1.6_f;
const fixed kYellowPikminThrowHeight = 2.0_f;

// Dsgx olimar_actor((u32*)olimar_dsgx, olimar_dsgx_size);
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
  body->is_movable = 1;
  body->collision_group = PLAYER_GROUP | WHISTLE_GROUP;
  body->sensor_groups = ONION_BEAM_GROUP;
  body->owner = &captain;

  //initialize our walking angle?
  captain.current_angle = 0_brad;

  //Initialize the cursor
  captain.cursor->set_actor(&cursor_actor);
  cursor_actor.ApplyTextures(captain.game->TextureAllocator(), captain.game->TexturePaletteAllocator());
  captain.cursor->body()->ignores_walls = 1;
  captain.cursor->body()->position = body->position
      + Vec3{0_f,0_f,5_f};
  captain.cursor->body()->is_sensor = 1;

  //Initialize the whistle
  captain.whistle->set_actor(&whistle_actor);
  whistle_actor.ApplyTextures(captain.game->TextureAllocator(), captain.game->TexturePaletteAllocator());
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
  // reset velocity in XZ to 0, so we stop moving
  // (but ignore Y so that we keep falling)
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
  captain.entity->RotateToFace(captain.current_angle, 10_brad);

  // Apply velocity in the direction of the current angle.
  auto body = captain.entity->body();
  body->velocity.x = trig::CosLerp(captain.current_angle);
  body->velocity.z = -trig::SinLerp(captain.current_angle);
  body->velocity.x *= kRunSpeed;
  body->velocity.z *= kRunSpeed;

  // Move the cursor in the same direction, at a faster rate
  auto cursor_body = captain.cursor->body();
  cursor_body->velocity.x = body->velocity.x * kCursorSpeedMultiplier;
  cursor_body->velocity.z = body->velocity.z * kCursorSpeedMultiplier;

  // Clamp the cursor to a certain distance from the captain
  Vec2 captain_xz = Vec2{body->position.x, body->position.z};
  Vec2 cursor_xz = Vec2{cursor_body->position.x, cursor_body->position.z};
  fixed distance = (cursor_xz - captain_xz).Length();
  if (distance > kCursorMaxDistance) {
    cursor_xz = (cursor_xz - captain_xz).Normalize() * kCursorMaxDistance;
    cursor_xz += captain_xz;
    cursor_body->position.x = cursor_xz.x;
    cursor_body->position.z = cursor_xz.y;
  }

  // Rotate the cursor so that it faces away from the captain
  captain.cursor->RotateToFace(captain.entity->AngleTo(captain.cursor));

  // Move the whistle to where the cursor is
  HandleWhistle(captain);

  // Handle collision with certain kinds of sensors
  if (captain.entity->body()->result_groups & ONION_BEAM_GROUP) {
    onion_ai::OnionState* current_onion =
        (onion_ai::OnionState*)(captain.entity->body()->FirstCollisionWith(ONION_BEAM_GROUP).body->owner);
    captain.active_onion = current_onion;
  } else {
    captain.active_onion = nullptr;
  }
  debug::DisplayValue("Pos: ", captain.entity->body()->position);
}

bool ActionDownNearPikmin(const CaptainState& captain) {
  if (keysDown() & KEY_A and captain.squad.squad_size > 0 and !(captain.active_onion)) {
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

  //Move the pikmin to olimar's hand
  auto pikmin_body = pikmin->entity->body();
  pikmin_body->position = captain.entity->body()->position;
  pikmin_body->position.x += trig::CosLerp(captain.current_angle);
  pikmin_body->position.y += 0.5_f;
  pikmin_body->position.z += -trig::SinLerp(captain.current_angle);
  pikmin->parent = captain.entity;
  captain.held_pikmin = pikmin;
}

void ThrowPikmin(CaptainState& captain) {
  captain.squad.RemovePikmin(captain.held_pikmin);

  fixed pikmin_y_velocity = kPikminThrowHeight;
  if (captain.held_pikmin->type == PikminType::kYellowPikmin) {
    pikmin_y_velocity = kYellowPikminThrowHeight;
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

bool RedButtonPressed(const CaptainState& captain) {
  if (keysDown() & KEY_TOUCH) {
    touchPosition touch;
    touchRead(&touch);

    if (touch.px < 40 and touch.py < 64) {
      return true;
    }
  }
  return false;
}

bool YellowButtonPressed(const CaptainState& captain) {
  if (keysDown() & KEY_TOUCH) {
    touchPosition touch;
    touchRead(&touch);

    if (touch.px < 40 and touch.py >= 64 and touch.py < 128) {
      return true;
    }
  }
  return false;
}

bool BlueButtonPressed(const CaptainState& captain) {
  if (keysDown() & KEY_TOUCH) {
    touchPosition touch;
    touchRead(&touch);

    if (touch.px < 40 and touch.py >= 128) {
      return true;
    }
  }
  return false;
}

template <PikminType T>
void SwitchTo(CaptainState& captain) {
  //un-hold the active pikmin
  captain.held_pikmin->parent = nullptr;

  //perform the sorting magic
  captain.squad.SortPikmin(T);

  //grab (again) the first pikmin in the squad
  GrabPikmin(captain);
}

bool DismissPressedWithSquad(const CaptainState& captain) {
  if ((keysDown() & KEY_X) and captain.squad.squad_size > 0) {
    return true;
  }
  return false;
}

void DismissSquad(CaptainState& captain) {
  Vec2 captain_positon = {
    captain.entity->body()->position.x,
    captain.entity->body()->position.z
  };

  Vec2 red_position = Vec2{9_f, 3_f}.Rotate(captain.entity->rotation().y)
      + captain_positon;
  Vec2 yellow_position = Vec2{0_f, 12_f}.Rotate(captain.entity->rotation().y)
      + captain_positon;
  Vec2 blue_position = Vec2{-9_f, 3_f}.Rotate(captain.entity->rotation().y)
      + captain_positon;

  while (captain.squad.squad_size > 0) {
    PikminState* pikmin = captain.squad.pikmin[0];
    if (pikmin->type == PikminType::kRedPikmin) {
      pikmin->target = red_position;
    }
    if (pikmin->type == PikminType::kYellowPikmin) {
      pikmin->target = yellow_position;
    }
    if (pikmin->type == PikminType::kBluePikmin) {
      pikmin->target = blue_position;
    }
    pikmin->has_target = true;
    captain.squad.RemovePikmin(pikmin);
  }
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
  {kAlways, DismissPressedWithSquad, DismissSquad, CaptainNode::kIdle},
  {kAlways, nullptr, IdleAlways, CaptainNode::kIdle},  // Loopback

  // Run
  {kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrabRun},
  {kAlways, DpadInactive, StopCaptain, CaptainNode::kIdle},
  {kAlways, DismissPressedWithSquad, DismissSquad, CaptainNode::kRun},
  {kAlways, nullptr, MoveCaptain, CaptainNode::kRun},  // Loopback

  // Grab
  {kAlways, ActionReleased, ThrowPikmin, CaptainNode::kThrow},
  {kAlways, DpadActive, MoveCaptain, CaptainNode::kGrabRun},
  {kAlways, RedButtonPressed, SwitchTo<PikminType::kRedPikmin>, CaptainNode::kGrab},
  {kAlways, YellowButtonPressed, SwitchTo<PikminType::kYellowPikmin>, CaptainNode::kGrab},
  {kAlways, BlueButtonPressed, SwitchTo<PikminType::kBluePikmin>, CaptainNode::kGrab},
  {kAlways, nullptr, IdleAlways, CaptainNode::kGrab},  // Loopback

  // GrabRun
  {kAlways, ActionReleased, ThrowPikmin, CaptainNode::kThrowRun},
  {kAlways, DpadInactive, StopCaptain, CaptainNode::kGrab},
  {kAlways, RedButtonPressed, SwitchTo<PikminType::kRedPikmin>, CaptainNode::kGrabRun},
  {kAlways, YellowButtonPressed, SwitchTo<PikminType::kYellowPikmin>, CaptainNode::kGrabRun},
  {kAlways, BlueButtonPressed, SwitchTo<PikminType::kBluePikmin>, CaptainNode::kGrabRun},
  {kAlways, nullptr, MoveCaptain, CaptainNode::kGrabRun},  // Loopback

  // Throw
  {kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrab},
  {kAlways, DpadActive, MoveCaptain, CaptainNode::kThrowRun},
  {kAlways, DismissPressedWithSquad, DismissSquad, CaptainNode::kIdle},
  {kLastFrame, nullptr, IdleAlways, CaptainNode::kIdle},
  {kAlways, nullptr, IdleAlways, CaptainNode::kThrow},  // Loopback

  // ThrowRun
  {kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrabRun},
  {kAlways, DpadInactive, StopCaptain, CaptainNode::kThrow},
  {kAlways, DismissPressedWithSquad, DismissSquad, CaptainNode::kRun},
  {kLastFrame, nullptr, MoveCaptain, CaptainNode::kRun},
  {kAlways, nullptr, MoveCaptain, CaptainNode::kThrowRun},  // Loopback
};

Node node_list[] {
  {"Init", true, 0, 0},
  {"Idle", true, 1, 4, "Armature|Idle1", 15},
  {"Run", true, 5, 8, "Armature|Run", 30},
  {"Grab", true, 9, 14, "Armature|Idle1", 15},
  {"GrabRun", true, 15, 20, "Armature|Run", 30},
  {"Throw", true, 21, 25, "Armature|Idle1", 5},
  {"ThrowRun", true, 26, 30, "Armature|Run", 5},
};

StateMachine<CaptainState> machine(node_list, edge_list);

}
