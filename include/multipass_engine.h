#ifndef MULTIPASS_ENGINE_H
#define MULTIPASS_ENGINE_H

#include <queue>

#include "drawable_entity.h"

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
  void drawEntity(DrawableEntity entity);

  void update();
  void draw();

  void addEntity(DrawableEntity* entity);

  static MultipassEngine* engine;

  int dPadDirection();
  int cameraAngle();

  void updateCamera();
  void setCamera(Vec3 position, Vec3 target);
  void targetEntity(DrawableEntity*);

 private:
  template <typename FixedT, int FixedF>
  using Fixed = numeric_types::Fixed<FixedT, FixedF>;

  void gatherDrawList();
  void setVRAMforPass(int pass);
  void applyCameraTransform();
  void drawClearPlane();

  std::priority_queue<EntityContainer> drawList;

  std::vector<DrawableEntity*> entities;
  std::vector<EntityContainer> overlap_list;
  std::vector<EntityContainer> pass_list;

  int current_pass = 0;

  bool debug_first_pass = false;
  bool debug_timings = false;
  bool debug_colors = false;

  int old_keys;
  int keys;
  int last_angle = 0;

  Fixed<s32, 12> near_plane;
  Fixed<s32, 12> far_plane;

  Vec3 camera_position_current;
  Vec3 camera_target_current;

  Vec3 camera_position_destination;
  Vec3 camera_target_destination;

  Vec3 camera_position_cached;
  Vec3 camera_target_cached;

  DrawableEntity* entity_to_follow;

  bool highCamera = false;
  int cameraDistance = 2;
};

#endif  // MULTIPASS_ENGINE_H
