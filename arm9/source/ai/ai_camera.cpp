#include "ai/ai_camera.h"
#include "ai/captain.h"
#include "numeric_types.h"
#include "pikmin_game.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

using captain_ai::CaptainState;

namespace camera_ai {

void InitAlways(CameraState& camera) {
  // Sensible Defaults
  camera.current_subject  = Vec3{0_f, 0_f, 10_f};
  camera.current_fov = 70_f;
  camera.target_subject = camera.current_subject;
  camera.target_fov = camera.current_fov;
  camera.zoom_step = 0_f;
}

void TweenValues(CameraState& camera) {
  camera.current_subject  = camera.current_subject  * 0.75_f + camera.target_subject  * 0.25_f;
  camera.current_height   = camera.current_height   * 0.75_f + camera.target_height   * 0.25_f;
  camera.current_fov      = camera.current_fov      * 0.75_f + camera.target_fov      * 0.25_f;

  // Angle needs some unusual considerations... this is lifted from the old camera implementation
  // and could likely be improved.
  // For the angle, we need to do fancy delta clamping
  auto delta = camera.target_angle - camera.current_angle;
  Brads max_turn_speed = 1_brad;

  // clamp the delta so that it is within -180, 180
  while (delta > 180_brad) {
    delta -= 360_brad;
  }
  while (delta < -180_brad) {
    delta += 360_brad;
  }

  // if the delta is greater than the rate, limit it for slow turning
  if (delta > max_turn_speed) {
    delta = delta / 4_f;
  }
  if (delta < -max_turn_speed) {
    delta = delta / 4_f;
  }

  camera.current_angle += delta;
}

void UpdateFollowCamera(CameraState& camera) {
  CaptainState* captain = camera.game->RetrieveCaptain(camera.follow_captain_entity);
  if (captain) {
    // Set our position to just above the Captain's current location
    camera.target_subject = captain->entity->position() + Vec3{0_f,2.5_f,0_f};
    TweenValues(camera);

    // Derive the camera's location from our current tweened values
    Vec3 camera_position{0_f,0_f,0_f};
    camera_position.x.data_ = -cosLerp(camera.current_angle.data_);
    camera_position.z.data_ = sinLerp(camera.current_angle.data_);
    camera_position.y = camera.current_height;
    camera_position = camera_position.Normalize();
    camera_position *= 45_f;

    camera_position += camera.current_subject;

    // Finally, write out the camera values to the renderer
    // TODO: THIS
  }
}

void UpdateLazyFollowCamera(CameraState& camera) {
  // not implemented!
  UpdateFollowCamera(camera);
}

bool FocusCursorPressed(const CameraState& camera) {
  return (keysDown() & KEY_L) or (keysHeld() & KEY_L);
}

bool FocusCursorReleased(const CameraState& camera) {
  return !(keysDown() & KEY_L) or (keysHeld() & KEY_L);
}

bool ZoomPressed(const CameraState& camera) {
  return (keysDown() & KEY_R) or (keysHeld() & KEY_R);
}

bool HeightPressed(const CameraState& camera) {
  return (keysDown() & KEY_X) or (keysHeld() & KEY_X);
}

void CenterBehindSubject(CameraState& camera) {
  CaptainState* captain = camera.game->RetrieveCaptain(camera.follow_captain_entity);
  if (captain) {
      camera.target_angle = captain->entity->AngleTo(captain->cursor);
  }
}

void IncrementZoomLevel(CameraState& camera) {
  camera.zoom_step = camera.zoom_step + 1_f;
  if (camera.zoom_step > 2_f) {
    camera.zoom_step = 0_f;
  }
  camera.target_fov = 70_f - (10_f * camera.zoom_step);
}

void ToggleHeightLevel(CameraState& camera) {
  camera.high_camera = !(camera.high_camera);
  if (camera.high_camera) {
    camera.target_height = 2.0_f;
  } else {
    camera.target_height = 0.5_f;
  }
}


namespace CameraNode {
enum CaptainNode {
  kInit = 0,
  kFollowCamera,
  kLazyFollowCamera,
};
}

Edge<CameraState> init[] {
  Edge<CameraState>{Trigger::kAlways, nullptr, InitAlways, CameraNode::kFollowCamera},
  END_OF_EDGES(CameraState)
};

Edge<CameraState> follow[] {
  Edge<CameraState>{Trigger::kAlways, FocusCursorPressed, CenterBehindSubject, CameraNode::kLazyFollowCamera},
  Edge<CameraState>{Trigger::kAlways, ZoomPressed, IncrementZoomLevel, CameraNode::kFollowCamera},
  Edge<CameraState>{Trigger::kAlways, HeightPressed, ToggleHeightLevel, CameraNode::kFollowCamera},
  Edge<CameraState>{Trigger::kAlways, nullptr, UpdateFollowCamera, CameraNode::kFollowCamera},
  END_OF_EDGES(CameraState)
};

Edge<CameraState> lazy_follow[] {
  Edge<CameraState>{Trigger::kAlways, FocusCursorReleased, nullptr, CameraNode::kFollowCamera},
  Edge<CameraState>{Trigger::kAlways, ZoomPressed, IncrementZoomLevel, CameraNode::kLazyFollowCamera},
  Edge<CameraState>{Trigger::kAlways, HeightPressed, ToggleHeightLevel, CameraNode::kLazyFollowCamera},
  Edge<CameraState>{Trigger::kAlways, nullptr, UpdateLazyFollowCamera, CameraNode::kLazyFollowCamera},
  END_OF_EDGES(CameraState)
};

Node<CameraState> node_list[] {
  {"Init", true, init},
  {"FollowCamera", true, follow},
  {"LazyFollowCamera", true, lazy_follow},
};

StateMachine<CameraState> machine(node_list);

}  // namespace camera_ai
