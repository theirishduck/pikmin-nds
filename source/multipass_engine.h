#ifndef MULTIPASS_ENGINE_H
#define MULTIPASS_ENGINE_H

#include <queue>

#include "drawable_entity.h"
#include "camera.h"

struct EntityContainer {
  template <typename FixedT, int FixedF>
  using Fixed = numeric_types::Fixed<FixedT, FixedF>;

  DrawableEntity* entity;
  Fixed<s32, 12> near_z;
  Fixed<s32, 12> far_z;
  bool operator<(const EntityContainer& other) const {
    return far_z < other.far_z;
  }
};

class MultipassEngine {
 public:
  MultipassEngine();
  void DrawEntity(DrawableEntity entity);

  void Update();
  void Draw();

  void AddEntity(DrawableEntity* entity);

  // TODO(Nick) This isn't being used anywhere. Should this be removed?
  static MultipassEngine* engine_;

  numeric_types::Brads DPadDirection();
  numeric_types::Brads CameraAngle();

  void TargetEntity(DrawableEntity*);

 private:
  template <typename FixedT, int FixedF>
  using Fixed = numeric_types::Fixed<FixedT, FixedF>;

  void GatherDrawList();
  void SetVRAMforPass(int pass);
  void ApplyCameraTransform();
  void DrawClearPlane();

  std::priority_queue<EntityContainer> draw_list_;

  std::vector<DrawableEntity*> entities_;
  std::vector<EntityContainer> overlap_list_;
  std::vector<EntityContainer> pass_list_;

  int current_pass_{0};

  bool debug_first_pass_{false};
  bool debug_timings_{false};
  bool debug_colors_{false};

  int old_keys_;
  int keys_;
  numeric_types::Brads last_angle_ = numeric_types::Brads::Raw(0);

  Fixed<s32, 12> near_plane_;
  Fixed<s32, 12> far_plane_;

  Camera camera;
};

#endif  // MULTIPASS_ENGINE_H
