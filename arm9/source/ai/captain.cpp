#include "captain.h"

#include <nds/arm9/input.h>
#include "dsgx.h"
#include "multipass_engine.h"

// Model data
#include "olimar_dsgx.h"
#include "olimar_low_poly_dsgx.h"

using numeric_types::literals::operator"" _f;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

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

void ReturnToIdle(CaptainState& captain) {
  //reset velocity to 0, so we stop moving
  captain.entity->body()->velocity = {0_f,0_f,0_f};
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
  captain.entity->body()->velocity.y = 0_f;
  captain.entity->body()->velocity.z.data_ = -sinLerp(captain.current_angle.data_);
  captain.entity->body()->velocity *= 0.2_f;
}

Edge<CaptainState> edge_list[] {
  Edge<CaptainState>{kAlways, nullptr, InitAlways, 1}, //Init -> Idle
  {kAlways,DpadActive,MoveCaptain,2},  // Idle -> Run
  {kAlways,DpadInactive,ReturnToIdle,1},  // Run -> Idle
  {kAlways,nullptr,MoveCaptain,2},  // Run -> Run (loopback, handles movement)
};

Node node_list[] {
  {"Init", true, 0, 0},
  {"Idle", true, 1, 1, "Armature|Idle1", 30},
  {"Run", true, 2, 3, "Armature|Run", 60},
};

StateMachine<CaptainState> machine(node_list, edge_list);

}