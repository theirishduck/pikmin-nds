#ifndef DRAWABLE_ENTITY_H
#define DRAWABLE_ENTITY_H

#include <string>

#include "dsgx.h"
#include "vector.h"
#include "physics/body.h"

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
  Dsgx* actor;
  Animation* animation{0};
  u32 animation_frame{0};
};

class MultipassEngine;

// Root for anything that the various Graphics Engines may use;
// intended to be inherited from to create game objects with
// custom logic.
class DrawableEntity {
 public:
  DrawableEntity();
  virtual ~DrawableEntity();
  DrawState& GetCachedState();
  void SetCache();

  Vec3 position();
  void set_position(Vec3);

  Rotation rotation();
  void set_rotation(numeric_types::Brads x, numeric_types::Brads y, numeric_types::Brads z);

  numeric_types::fixed scale();
  void set_scale(numeric_types::fixed new_scale);

  void RotateToXZDirection(Vec2 direction);
  numeric_types::Brads AngleTo(const DrawableEntity* destination);
  void RotateToFace(numeric_types::Brads target_angle, numeric_types::Brads rate = numeric_types::Brads::Raw(degreesToAngle(180)));
  void RotateToFace(const DrawableEntity* destination, numeric_types::Brads rate = numeric_types::Brads::Raw(degreesToAngle(180)));

  void set_actor(Dsgx* actor);
  Dsgx* actor();

  physics::Body* body();

  void set_engine(MultipassEngine* engine);
  MultipassEngine* engine();

  virtual void Update();
  virtual void Init();
  inline void ApplyTransformation();
  void Draw();

  numeric_types::fixed GetRealModelZ();

  void SetAnimation(std::string name);

  bool important{true};

 private:
  DrawState current_;
  DrawState cached_;

  s32 cached_matrix_[13]; //one extra entry for size; for DMA transfers

  MultipassEngine* engine_{nullptr};
 protected:
  physics::Body* body_{nullptr};
};

#endif  // DRAWABLE_ENTITY_H
