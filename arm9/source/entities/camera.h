#ifndef CAMERA_H
#define CAMERA_H

#include "numeric_types.h"
#include "vector.h"

class DrawableEntity;

struct CameraState {
  Vec3 target{numeric_types::fixed::FromInt(0), numeric_types::fixed::FromInt(6), numeric_types::fixed::FromInt(4)};
  numeric_types::Brads angle{numeric_types::Brads::Raw(0)};
  numeric_types::fixed height{numeric_types::fixed::FromFloat(0.5f)};
  numeric_types::fixed distance{numeric_types::fixed::FromInt(17)};
};

class Camera {
 public:
  Camera();
  void LookAt(Vec3 position, Vec3 target, bool instant = false);
  void FollowEntity(DrawableEntity* target);

  numeric_types::Brads GetAngle();

  void Update();
  void ApplyTransform();
  void SetCache();

 private:
  DrawableEntity* target_;

  CameraState target_state_;
  CameraState current_state_;
  CameraState cached_state_; 

  DrawableEntity* entity_to_follow_;
};

#endif  // CAMERA_H
