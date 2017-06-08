#ifndef DRAWABLE_ENTITY_H
#define DRAWABLE_ENTITY_H

#include <string>

#include "dsgx.h"
#include "vector.h"

struct Rotation {
  numeric_types::Brads x;
  numeric_types::Brads y;
  numeric_types::Brads z;
};

struct DrawState {
  Vec3 position;
  Rotation rotation;
  numeric_types::fixed scale;

  // TODO(Nick) try making this reference an animation state instead.
  Dsgx* actor{nullptr};
  Mesh* current_mesh;
  Animation* animation{0};
  u32 animation_frame{0};
};

// Root for anything that the various Graphics Engines may use;
// intended to be inherited from to create game objects with
// custom logic.
class Drawable {
 public:
  Drawable();
  DrawState& GetCachedState();
  void SetCache();

  Vec3 position() const;
  void set_position(Vec3);

  Rotation rotation() const;
  void set_rotation(numeric_types::Brads x, numeric_types::Brads y, numeric_types::Brads z);
  void set_rotation(Rotation rotation);

  numeric_types::fixed scale() const;
  void set_scale(numeric_types::fixed new_scale);

  numeric_types::Brads AngleTo(const Drawable* destination);
  void RotateToFace(numeric_types::Brads target_angle, numeric_types::Brads rate = numeric_types::Brads::Raw(degreesToAngle(180)));
  void RotateToFace(const Drawable* destination, numeric_types::Brads rate = numeric_types::Brads::Raw(degreesToAngle(180)));

  bool InsideViewFrustrum();

  void set_actor(Dsgx* actor);
  Dsgx* actor();
  void set_mesh(const char* mesh_name);
  Mesh* mesh();

  void Update();
  inline void ApplyTransformation();
  void Draw();

  numeric_types::fixed GetRealModelZ();

  void SetAnimation(std::string name);

  bool important{true};
  unsigned int overlaps{0};
  bool visible{false};

 private:
  DrawState current_{};
  DrawState cached_{};

  s32 cached_matrix_[13]; //one extra entry for size; for DMA transfers
};

#endif  // DRAWABLE_ENTITY_H
