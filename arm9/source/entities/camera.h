#ifndef CAMERA_H
#define CAMERA_H

#include "numeric_types.h"
#include "vector.h"

namespace captain_ai {
class CaptainState;
}

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
  void FollowCaptain(captain_ai::CaptainState* target);

  numeric_types::Brads GetAngle();

  void Update();
  void ApplyTransform();
  void SetCache();
  Vec3 Position();
  Vec3 Target();

 private:
  captain_ai::CaptainState* target_;

  CameraState target_state_;
  CameraState current_state_;
  CameraState cached_state_; 

  captain_ai::CaptainState* captain_to_follow_;
};

#endif  // CAMERA_H
