#include "camera.h"

#include <cstdio>

#include <nds.h>

#include "drawable_entity.h"

using namespace std;
using numeric_types::literals::operator"" _f;
using numeric_types::fixed;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

Camera::Camera() {
  position_destination_ = Vec3{0_f, 6_f, 4_f};
  target_destination_  = Vec3{0_f, 3_f, 0_f};

  position_current_ = position_destination_;
  target_current_ = target_destination_;

  SetCache();
}

void Camera::Update() {
  if (keysDown() & KEY_R) {
    if (keysHeld() & KEY_L) {
      distance_ += 1;
      if (distance_ > 3) {
        distance_ = 1;
      }
    } else {
      high_camera_ = not high_camera_;
    }
  }

  if (entity_to_follow_) {
    fixed const height = (high_camera_ ? 7.5_f : 2.5_f) *
        fixed::FromInt(distance_ + 1);

    if (keysDown() & KEY_L) {
      // Move the camera directly behind the target entity, based on their
      // current rotation.
      position_destination_ = entity_to_follow_->position();
      position_destination_.x.data_ -=
          cosLerp((entity_to_follow_->rotation().y - 90_brad).data_);
      position_destination_.z.data_ -=
          -sinLerp((entity_to_follow_->rotation().y - 90_brad).data_);
    }

    fixed follow_distance = 4.0_f + 6.0_f * fixed::FromInt(distance_);

    target_destination_ = entity_to_follow_->position();
    Vec3 entity_to_camera = entity_to_follow_->position() -
        position_destination_;
    entity_to_camera.y = 0_f;  // Clear out height to work in the XZ plane.
    entity_to_camera = entity_to_camera.Normalize();
    entity_to_camera = entity_to_camera * follow_distance;
    position_destination_ = entity_to_follow_->position() - entity_to_camera;
    position_destination_.y = height;

    printf("\x1b[8;0HC. Position: %.1f, %.1f, %.1f\n",
        (float)position_destination_.x, (float)position_destination_.y,
        (float)position_destination_.z);
    printf("C. Target  : %.1f, %.1f, %.1f\n", (float)target_destination_.x,
        (float)target_destination_.y, (float)target_destination_.z);
  } else {
    printf("No entity?\n");
  }

  // Take the weighted average for position and target to take a smooth
  // transition between the old value and the new one.
  position_current_ = position_destination_ * 0.25_f + position_current_ *
      0.75_f;
  target_current_ = target_destination_ * 0.25_f + target_current_ * 0.75_f;
}

void Camera::LookAt(Vec3 position, Vec3 target, bool instant) {
  position_destination_ = position;
  target_destination_ = target;
  if (instant) {
    position_current_ = position;
    target_current_ = target;
  }
}

Brads Camera::GetAngle() {
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
  }
}

void Camera::FollowEntity(DrawableEntity* entity) {
  entity_to_follow_ = entity;
}

// TODO(Nick) Convert this to non-float implementation.
void Camera::ApplyTransform() {
  gluLookAt(
      (float)position_cached_.x, (float)position_cached_.y,
      (float)position_cached_.z, (float)target_cached_.x,
      (float)target_cached_.y, (float)target_cached_.z, 0.0f, 1.0f, 0.0f);
}

void Camera::SetCache() {
  position_cached_ = position_current_;
  target_cached_ = target_current_;
}
