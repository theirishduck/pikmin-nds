#include "camera.h"
#include "project_settings.h"

#include <cstdio>

#include <nds.h>

#include "ai/captain.h"
#include "pikmin_game.h"
#include "debug/utilities.h"

using namespace std;
using numeric_types::literals::operator"" _f;
using numeric_types::fixed;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

Camera::Camera() {
  SetCache();
}

Brads AngleBetween(const Vec3 source_position, const Vec3 destination_position) {
  auto difference = Vec2{destination_position.x, destination_position.z} -
      Vec2{source_position.x, source_position.z};
  if (difference.Length2() > 0_f) {
    difference = difference.Normalize();
    if (difference.y <= 0_f) {
      return Brads::Raw(acosLerp(difference.x.data_));
    }
    return Brads::Raw(-acosLerp(difference.x.data_));
  }
  return 0_brad;
}

void Camera::Update() {
  if (target_->game->IsPaused()) {
    return;
  }
  if (keysDown() & KEY_R) {
    if (keysHeld() & KEY_L) {
      target_state_.distance += CAMERA_STEP;
      if (target_state_.distance > CAMERA_CLOSEST + (CAMERA_STEP * 2_f)) {
        target_state_.distance = CAMERA_CLOSEST;
      }
    } else {
      if (target_state_.height == CAMERA_LOW_HEIGHT) {
        target_state_.height = CAMERA_HIGH_HEIGHT;
      } else {
        target_state_.height = CAMERA_LOW_HEIGHT;
      }
    }
  }

  if (captain_to_follow_) {
    if (keysDown() & KEY_L) {
      // Move the camera directly behind the target entity, based on their
      // current rotation.

      //target_state_.angle = captain_to_follow_->entity->rotation().y - 90_brad;
      target_state_.angle = captain_to_follow_->entity->AngleTo(captain_to_follow_->cursor);
    }
    if (keysHeld() & KEY_L) {
      // Adjust the rotation based on the new position of the entity, to
      // achieve kind of a lazy follow and rotate
      auto angle_to_current_target = AngleBetween(Position(), target_state_.target);
      auto angle_to_new_target = AngleBetween(Position(), captain_to_follow_->entity->position());
      auto delta = angle_to_new_target - angle_to_current_target;

      // clamp the delta so that it is within -180, 180
      while (delta > 180_brad) {
        delta -= 360_brad;
      }
      while (delta < -180_brad) {
        delta += 360_brad;
      }

      // dampen to make the camera more lazy
      delta = delta / 2_f;
      target_state_.angle += delta;
    }
    target_state_.target = captain_to_follow_->entity->position() + Vec3{0_f,2.5_f,0_f};
  } else {
    //printf("No entity?\n");
  }

  // Take the weighted average for position and target to take a smooth
  // transition between the old value and the new one.

  current_state_.target = current_state_.target * 0.75_f
      + target_state_.target * 0.25_f;

  current_state_.height = current_state_.height * 0.75_f
      + target_state_.height * 0.25_f;

  current_state_.distance = current_state_.distance * 0.75_f
      + target_state_.distance * 0.25_f;

  // For the angle, we need to do fancy delta clamping
  auto delta = target_state_.angle - current_state_.angle;
  Brads max_jump = 1_brad;

  // clamp the delta so that it is within -180, 180
  while (delta > 180_brad) {
    delta -= 360_brad;
  }
  while (delta < -180_brad) {
    delta += 360_brad;
  }

  // if the delta is greater than the rate, limit it for slow turning
  if (delta > max_jump) {
    delta = delta / 4_f;
  }
  if (delta < -max_jump) {
    delta = delta / 4_f;
  }

  current_state_.angle += delta;
}

void Camera::LookAt(Vec3 position, Vec3 target, bool instant) {
  /*
  position_destination_ = position;
  target_destination_ = target;
  if (instant) {
    position_current_ = position;
    target_current_ = target;
  }*/
}

Brads Camera::GetAngle() {
  /*
  Vec3 facing;
  facing = entity_to_follow_->position() - position_current_;
  facing.y = 0_f;  // Work on the XZ plane.
  if (facing.Length() <= 0_f) {
    return 0_brad;
  }
  facing = facing.Normalize();

  // return 0;
  if (facing.z <= 0_f) {
    return Brads::Raw(acosLerp(facing.x.data_));
  } else {
    return Brads::Raw(-acosLerp(facing.x.data_));
  }*/
  return current_state_.angle;
}

void Camera::FollowCaptain(captain_ai::CaptainState* target) {
  captain_to_follow_ = target;
}

Vec3 Camera::Position() {
  Vec3 offset{0_f,0_f,0_f};
  offset.x.data_ = -cosLerp(cached_state_.angle.data_);
  offset.z.data_ = sinLerp(cached_state_.angle.data_);
  offset.y = cached_state_.height;
  offset = offset.Normalize();
  offset *= cached_state_.distance + cached_state_.height;

  return cached_state_.target + offset;
}

Vec3 Camera::Target() {
  return cached_state_.target;
}

// TODO(Nick) Convert this to non-float implementation.
void Camera::ApplyTransform() {
  // With our camera parameters, generate a position and a target for
  // OpenGL's LookAt function
  Vec3 position = Position();
  Vec3 target = Target();

  gluLookAt(
      (float)position.x, (float)position.y,
      (float)position.z, (float)target.x,
      (float)target.y, (float)target.z, 0.0f, 1.0f, 0.0f);
}

void Camera::SetCache() {
  cached_state_ = current_state_;
}
