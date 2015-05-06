#include "camera.h"

#include <cstdio>

#include <nds.h>

#include "drawable_entity.h"
#include "debug.h"

using namespace std;
using numeric_types::literals::operator"" _f;
using numeric_types::fixed;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

Camera::Camera() {
  SetCache();
}

void Camera::Update() {
  if (keysDown() & KEY_R) {
    if (keysHeld() & KEY_L) {
      target_state_.distance += 7_f;
      if (target_state_.distance > 24_f) {
        target_state_.distance = 10_f;
      }
    } else {
      if (target_state_.height == 0.5_f) {
        target_state_.height = 2_f;
      } else {
        target_state_.height = 0.5_f;
      }
    }
  }

  if (entity_to_follow_) {
    if (keysDown() & KEY_L) {
      // Move the camera directly behind the target entity, based on their
      // current rotation.
      target_state_.angle = entity_to_follow_->rotation().y - 90_brad;
    }
    target_state_.target = entity_to_follow_->position() + Vec3{0_f,2.5_f,0_f};
  } else {
    //printf("No entity?\n");
  }

  // Take the weighted average for position and target to take a smooth
  // transition between the old value and the new one.
  
  current_state_.target = current_state_.target * 0.875_f
      + target_state_.target * 0.125_f;

  current_state_.height = current_state_.height * 0.875_f
      + target_state_.height * 0.125_f;

  current_state_.distance = current_state_.distance * 0.875_f
      + target_state_.distance * 0.125_f;

  // For the angle, we need to do fancy delta clamping
  //TODO: that later
  current_state_.angle = target_state_.angle;

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

void Camera::FollowEntity(DrawableEntity* entity) {
  entity_to_follow_ = entity;
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
