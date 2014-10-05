#include "Captain.h"

#include <nds/arm9/input.h>

#include "dsgx.h"
#include "multipass_engine.h"

// Model data
#include "olimar_dsgx.h"
#include "olimar_low_poly_dsgx.h"
#include "test_dsgx.h"

using numeric_types::literals::operator"" _f;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

Captain::Captain() {
  // Todo(Nick) Share Dsgx instances across Captain instances.
  Dsgx* olimar_actor = new Dsgx((u32*)olimar_low_poly_dsgx, olimar_low_poly_dsgx_size);
  set_actor(olimar_actor);
  SetAnimation("Armature|Idle1");
}

Captain::~Captain() {
  // Todo(Nick) Use an asset loader to deallocate model data.
  delete actor();
}

void Captain::Update(MultipassEngine* engine) {
  if (running_) {
    if (!(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))) {
      running_ = false;
      SetAnimation("Armature|Idle1");
    }
  } else {
    if (keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
      running_ = true;
      SetAnimation("Armature|Run");
    }
  }

  if (running_) {
    Brads dpad_angle = engine->CameraAngle() + engine->DPadDirection() -
        90_brad;
    Brads delta = dpad_angle - current_angle_;

    // Translate delta to be in +/-180 degrees, then clamp it to limit the
    // maximum turning angle per frame.
    // Todo(Cristian) This may not be necessary anymore due to the use of Brads.
    while (delta >= 180_brad) {
      delta -= 360_brad;
    }
    while (delta < -180_brad) {
      delta += 360_brad;
    }
    if (delta > 11_brad) {
      delta = 11_brad;
    }
    if (delta < -11_brad) {
      delta = -11_brad;
    }

    current_angle_ += delta;
    // Ensure current_angle_ is within +/-180 degrees.
    if (current_angle_ >= 360_brad) {
      current_angle_ -= 360_brad;
    }
    if (current_angle_ < 0_brad) {
      current_angle_ += 360_brad;
    }

    set_rotation(0_brad, current_angle_ + 90_brad, 0_brad);

    // Apply velocity in the direction of the current angle.
    Vec3 movement;
    movement.x.data_ = cosLerp(current_angle_.data_);
    movement.z.data_ = -sinLerp(current_angle_.data_);
    set_position(position() + 
        movement * 0.2_f);
  }

  DrawableEntity::Update(engine);
}
