#ifndef AI_CAMERA_H
#define AI_CAMERA_H

#include "handle.h"
#include "numeric_types.h"
#include "state_machine.h"
#include "vector.h"

class PikminGame;

namespace camera_ai {

struct CameraState : ObjectState {
  PikminGame* game;

  Vec3 current_subject;
  numeric_types::fixed current_height;
  numeric_types::Brads current_angle;
  numeric_types::fixed current_fov;

  Vec3 target_subject;
  numeric_types::fixed target_height;
  numeric_types::Brads target_angle;
  numeric_types::fixed target_fov;

  bool high_camera{false};
  numeric_types::fixed zoom_step;

  Handle follow_captain_entity;
};

extern StateMachine<CameraState> machine;

}  // namespace camera_ai

#endif
