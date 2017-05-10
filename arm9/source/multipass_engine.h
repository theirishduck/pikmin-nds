#ifndef MULTIPASS_ENGINE_H
#define MULTIPASS_ENGINE_H

#include <queue>
#include <list>
#include <map>
#include <string>

#include "drawable_entity.h"
#include "entities/camera.h"
#include "physics/world.h"
#include "debug/profiler.h"

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
  void RemoveEntity(DrawableEntity* entity);

  numeric_types::Brads DPadDirection();
  numeric_types::Brads CameraAngle();

  Camera* camera();

  unsigned int FrameCounter();
  void PauseEngine();
  void UnpauseEngine();
  bool IsPaused();

  debug::Profiler& DebugProfiler();

  void EnableEffectsLayer(bool enabled);

  std::map<std::string, bool> debug_flags;
 private:
  bool paused_ = false;
  template <typename FixedT, int FixedF>
  using Fixed = numeric_types::Fixed<FixedT, FixedF>;

  void GatherDrawList();
  void ClearDrawList();
  void SetVRAMforPass(int pass);
  void ApplyCameraTransform();
  void DrawClearPlane();

  void InitFrame();
  void GatherPassList();
  bool ProgressMadeThisPass(unsigned int initial_length);
  void SetupDividingPlane();
  bool ValidateDividingPlane();
  void DrawPassList();
  bool LastPass();
  void DrawEffects();

  std::priority_queue<EntityContainer> draw_list_;

  std::list<DrawableEntity*> entities_;
  std::vector<EntityContainer> overlap_list_;
  std::vector<EntityContainer> pass_list_;

  int current_pass_{0};

  int old_keys_;
  int keys_;
  numeric_types::Brads last_angle_ = numeric_types::Brads::Raw(0);

  Fixed<s32, 12> near_plane_;
  Fixed<s32, 12> far_plane_;

  Camera camera_;

  unsigned int frame_counter_{0};

  bool effects_enabled{false};
  bool effects_drawn{false};

  debug::Profiler debug_profiler_;

  // Debug Topics
  int tEntityUpdate;
  int tParticleUpdate;
  int tParticleDraw;
  int tFrameInit;
  int tPassInit;
  int tIdle;
  std::vector<int> tPassUpdate;
};

#endif  // MULTIPASS_ENGINE_H
