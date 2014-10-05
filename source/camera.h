#ifndef CAMERA_H
#define CAMERA_H

#include "numeric_types.h"
#include "vector.h"

class DrawableEntity;

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

  Vec3 position_current_;
  Vec3 target_current_;

  Vec3 position_destination_;
  Vec3 target_destination_;

  Vec3 position_cached_;
  Vec3 target_cached_;

  DrawableEntity* entity_to_follow_;

  bool high_camera_{false};
  int distance_{2};
};

#endif  // CAMERA_H
