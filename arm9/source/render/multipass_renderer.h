#ifndef MULTIPASS_RENDERER_H
#define MULTIPASS_RENDERER_H

#include <list>
#include <queue>

#include "debug/profiler.h"
#include "render/strategy.h"
#include "render/back_to_front.h"
#include "numeric_types.h"
#include "vector.h"

class Drawable;

struct EntityContainer {

  Drawable* entity;
  numeric_types::fixed near_z;
  numeric_types::fixed far_z;
  bool operator<(const EntityContainer& other) const {
    return far_z < other.far_z;
  }
};

class MultipassRenderer {
 public:
  MultipassRenderer();

  void Update();
  void Draw();

  void AddEntity(Drawable* entity);
  void RemoveEntity(Drawable* entity);

  void PauseEngine();
  void UnpauseEngine();
  bool IsPaused();

  void SetCamera(Vec3 position, Vec3 subject, numeric_types::Brads fov);

  void EnableEffectsLayer(bool enabled);
  void DebugCircles();

 private:
  friend class render::Strategy;
  friend class render::BackToFront;
  void InitializeRender();

  void ClearDrawList();
  void SetVRAMforPass(int pass);
  void DrawClearPlane();
  void BailAndResetFrame();

  void CacheCamera();
  void ApplyCameraTransform();

  void GatherPassList();
  bool ProgressMadeThisPass(unsigned int initial_length);
  void SetupDividingPlane();
  bool ValidateDividingPlane();
  void DrawPassList();
  bool LastPass();
  void DrawEffects();

  void WaitForVBlank();

  void ClipFriendlyPerspective(numeric_types::fixed near, numeric_types::fixed far, numeric_types::Brads angle);

  render::Strategy* current_strategy_;
  bool paused_ = false;

  std::list<Drawable*> entities_;

  std::priority_queue<EntityContainer> draw_list_;
  std::vector<EntityContainer> overlap_list_;
  std::vector<EntityContainer> pass_list_;

  int current_pass_{0};

  numeric_types::fixed near_plane_;
  numeric_types::fixed far_plane_;

  Vec3 current_camera_position_;
  Vec3 current_camera_subject_;
  numeric_types::Brads current_camera_fov_;

  Vec3 cached_camera_position_;
  Vec3 cached_camera_subject_;
  numeric_types::Brads cached_camera_fov_;

  unsigned int frame_counter_{0};

  bool effects_enabled{false};
  bool effects_drawn{false};

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
