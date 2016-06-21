#include "captain.h"

#include <nds/arm9/input.h>
#include "dsgx.h"
#include "multipass_engine.h"
#include "pikmin_game.h"
#include "trig.h"
#include "ai/onion.h"

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
const fixed kPikminThrowHeight = 1.8_f;
const fixed kYellowPikminThrowHeight = 2.2_f;
const int kWhistleExpandFrames = 8;

void HandleWhistle(CaptainState& captain) {
  auto whistle_body = captain.whistle->body_handle().body;
  auto cursor_body = captain.cursor->body_handle().body;
  whistle_body->position = cursor_body->position;

  // Do a bit of cheating and handle the whistle here for now
  if (keysHeld() & KEY_B) {
    if (captain.whistle_timer < 8) {
      captain.whistle_timer++;
      //turn the whistle on
      whistle_body->collision_group = WHISTLE_GROUP;
    }
  } else if (captain.whistle_timer > 0) {
    captain.whistle_timer--;
    //turn the whistle back off
    whistle_body->collision_group = 0;
  }

  whistle_body->radius = fixed::FromInt(captain.whistle_timer) * 10_f / fixed::FromInt(kWhistleExpandFrames);
  captain.whistle->set_scale(fixed::FromInt(captain.whistle_timer) * 10_f / fixed::FromInt(kWhistleExpandFrames));
  captain.whistle->set_rotation(0_brad, captain.whistle->rotation().y + 3_brad, 0_brad);
}

void InitAlways(CaptainState& captain) {
  //set the actor for animation
  captain.entity->set_actor(captain.game->ActorAllocator()->Retrieve("olimar_low_poly"));
  // captain.entity->set_mesh("Olimar");
  captain.entity->set_mesh("MediumPolyOlimar");

  //setup physics parameters for collision
  captain.body->height = 6_f;
  captain.body->radius = 1.5_f;

  captain.body->collides_with_bodies = 1;
  captain.body->is_movable = 1;
  captain.body->collision_group = PLAYER_GROUP | WHISTLE_GROUP;
  captain.body->sensor_groups = ONION_BEAM_GROUP;
  captain.body->owner = &captain;

  //initialize our walking angle?
  captain.current_angle = 0_brad;

  //Initialize the cursor
  captain.cursor->set_actor(captain.game->ActorAllocator()->Retrieve("cursor"));
  auto cursor_body = captain.cursor->body_handle().body;
  cursor_body->ignores_walls = 1;
  cursor_body->position = captain.body->position + Vec3{0_f,0_f,5_f};
  cursor_body->is_sensor = 1;

  //Initialize the whistle
  captain.whistle->set_actor(captain.game->ActorAllocator()->Retrieve("whistle"));
  auto whistle_body = captain.whistle->body_handle().body;

  whistle_body->height = 20.0_f;
  whistle_body->is_sensor = 1;
  whistle_body->owner = &captain;
  whistle_body->is_very_important = 1;
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
  captain.body->velocity.x = 0_f;
  captain.body->velocity.z = 0_f;
  auto cursor_body = captain.cursor->body_handle().body;
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
  captain.entity->RotateToFace(captain.current_angle, 20_brad);

  // Apply velocity in the direction of the current angle.
  captain.body->velocity.x = trig::CosLerp(captain.current_angle);
  captain.body->velocity.z = -trig::SinLerp(captain.current_angle);
  captain.body->velocity.x *= kRunSpeed;
  captain.body->velocity.z *= kRunSpeed;

  // Move the cursor in the same direction, at a faster rate
  auto cursor_body = captain.cursor->body_handle().body;
  cursor_body->velocity.x = captain.body->velocity.x * kCursorSpeedMultiplier;
  cursor_body->velocity.z = captain.body->velocity.z * kCursorSpeedMultiplier;

  // Clamp the cursor to a certain distance from the captain
  Vec2 captain_xz = Vec2{captain.body->position.x, captain.body->position.z};
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
  if (captain.entity->body_handle().body->result_groups & ONION_BEAM_GROUP) {
    onion_ai::OnionState* current_onion =
        (onion_ai::OnionState*)(captain.entity->body_handle().body->FirstCollisionWith(ONION_BEAM_GROUP).body->owner);
    captain.active_onion = current_onion;
  } else {
    captain.active_onion = nullptr;
  }
  debug::DisplayValue("Pos: ", captain.position());
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
  pikmin->set_position(captain.position() + Vec3{
    trig::CosLerp(captain.current_angle),
    0.5_f,
    -trig::SinLerp(captain.current_angle)
  });
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
  Vec3 distance_to_cursor = captain.cursor->body_handle().body->position -
      captain.position();

  fixed pikmin_x_velocity = distance_to_cursor.x / pikmin_travel_time;
  fixed pikmin_z_velocity = distance_to_cursor.z / pikmin_travel_time;

  // Add in the captain's velocity; this is an intended gameplay mechanic
  pikmin_x_velocity += captain.velocity().x;
  pikmin_z_velocity += captain.velocity().z;

  captain.held_pikmin->set_velocity(Vec3{
      pikmin_x_velocity, pikmin_y_velocity, pikmin_z_velocity});
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
    captain.position().x,
    captain.position().z
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

Edge<CaptainState> init[] {
  Edge<CaptainState>{Trigger::kAlways, nullptr, InitAlways, CaptainNode::kIdle},
  END_OF_EDGES(CaptainState)
};

Edge<CaptainState> idle[] {
  {Trigger::kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrab},
  {Trigger::kAlways, DpadActive, MoveCaptain, CaptainNode::kRun},
  {Trigger::kAlways, DismissPressedWithSquad, DismissSquad, CaptainNode::kIdle},
  {Trigger::kAlways, nullptr, IdleAlways, CaptainNode::kIdle},  // Loopback
  END_OF_EDGES(CaptainState)
};

Edge<CaptainState> running[] {
  {Trigger::kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrabRun},
  {Trigger::kAlways, DpadInactive, StopCaptain, CaptainNode::kIdle},
  {Trigger::kAlways, DismissPressedWithSquad, DismissSquad, CaptainNode::kRun},
  {Trigger::kAlways, nullptr, MoveCaptain, CaptainNode::kRun},  // Loopback
  END_OF_EDGES(CaptainState)
};

Edge<CaptainState> grab[] {
  {Trigger::kAlways, ActionReleased, ThrowPikmin, CaptainNode::kThrow},
  {Trigger::kAlways, DpadActive, MoveCaptain, CaptainNode::kGrabRun},
  {Trigger::kAlways, RedButtonPressed, SwitchTo<PikminType::kRedPikmin>, CaptainNode::kGrab},
  {Trigger::kAlways, YellowButtonPressed, SwitchTo<PikminType::kYellowPikmin>, CaptainNode::kGrab},
  {Trigger::kAlways, BlueButtonPressed, SwitchTo<PikminType::kBluePikmin>, CaptainNode::kGrab},
  {Trigger::kAlways, nullptr, IdleAlways, CaptainNode::kGrab},  // Loopback
  END_OF_EDGES(CaptainState)
};

Edge<CaptainState> grab_run[] {
  {Trigger::kAlways, ActionReleased, ThrowPikmin, CaptainNode::kThrowRun},
  {Trigger::kAlways, DpadInactive, StopCaptain, CaptainNode::kGrab},
  {Trigger::kAlways, RedButtonPressed, SwitchTo<PikminType::kRedPikmin>, CaptainNode::kGrabRun},
  {Trigger::kAlways, YellowButtonPressed, SwitchTo<PikminType::kYellowPikmin>, CaptainNode::kGrabRun},
  {Trigger::kAlways, BlueButtonPressed, SwitchTo<PikminType::kBluePikmin>, CaptainNode::kGrabRun},
  {Trigger::kAlways, nullptr, MoveCaptain, CaptainNode::kGrabRun},  // Loopback
  END_OF_EDGES(CaptainState)
};

Edge<CaptainState> throw_pikmin[] {
  {Trigger::kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrab},
  {Trigger::kAlways, DpadActive, MoveCaptain, CaptainNode::kThrowRun},
  {Trigger::kAlways, DismissPressedWithSquad, DismissSquad, CaptainNode::kIdle},
  {Trigger::kLastFrame, nullptr, IdleAlways, CaptainNode::kIdle},
  {Trigger::kAlways, nullptr, IdleAlways, CaptainNode::kThrow},  // Loopback
  END_OF_EDGES(CaptainState)
};

Edge<CaptainState> throw_pikmin_while_running[] {
  {Trigger::kAlways, ActionDownNearPikmin, GrabPikmin, CaptainNode::kGrabRun},
  {Trigger::kAlways, DpadInactive, StopCaptain, CaptainNode::kThrow},
  {Trigger::kAlways, DismissPressedWithSquad, DismissSquad, CaptainNode::kRun},
  {Trigger::kLastFrame, nullptr, MoveCaptain, CaptainNode::kRun},
  {Trigger::kAlways, nullptr, MoveCaptain, CaptainNode::kThrowRun},  // Loopback
  END_OF_EDGES(CaptainState)
};

Node<CaptainState> node_list[] {
  {"Init", true, init},
  {"Idle", true, idle, "Armature|Idle1", 15},
  {"Run", true, running, "Armature|Run", 30},
  {"Grab", true, grab, "Armature|Idle1", 15},
  {"GrabRun", true, grab_run, "Armature|Run", 30},
  {"Throw", true, throw_pikmin, "Armature|Idle1", 5},
  {"ThrowRun", true, throw_pikmin_while_running, "Armature|Run", 5},
};

StateMachine<CaptainState> machine(node_list);

}  // namespace captain_ai
