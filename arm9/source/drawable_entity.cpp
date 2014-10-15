#include "drawable_entity.h"

#include <cstdio>

#include <nds/arm9/postest.h>

#include "multipass_engine.h"

namespace nt = numeric_types;

Vec3 DrawableEntity::position() {
  return current_.position;
}

void DrawableEntity::set_position(Vec3 pos) {
  current_.position = pos;
}

Rotation DrawableEntity::rotation() {
  return current_.rotation;
}

void DrawableEntity::set_rotation(nt::Brads x, nt::Brads y, nt::Brads z) {
  current_.rotation.x = x;
  current_.rotation.y = y;
  current_.rotation.z = z;
}

DrawState& DrawableEntity::GetCachedState() {
  return cached_;
}

void DrawableEntity::SetCache() {
  cached_ = current_;
}

void DrawableEntity::set_actor(Dsgx* actor) {
  current_.actor = actor;
}

Dsgx* DrawableEntity::actor() {
  return current_.actor;
}

void DrawableEntity::set_engine(MultipassEngine* engine) {
  engine_ = engine;
}

MultipassEngine* DrawableEntity::engine() {
  return engine_;
}

void DrawableEntity::ApplyTransformation() {
  glTranslatef32(cached_.position.x.data_, cached_.position.y.data_,
      cached_.position.z.data_);

  // If the rotation value is zero, skip the gl call; this doesn't affect the
  // end result, but does skip an avoidable expensive matrix transformation.
  // Most rotations will only be about the Y axis, meaning that the X/Z axes are
  // often skipped. Initial testing shows this reducing CPU load of this
  // function by ~1/2 for typical scenes.
  if (cached_.rotation.y.data_) {
    glRotateYi(cached_.rotation.y.data_);
  }
  if (cached_.rotation.x.data_) {
    glRotateXi(cached_.rotation.x.data_);
  }
  if (cached_.rotation.z.data_) {
    glRotateZi(cached_.rotation.z.data_);
  }
}

void DrawableEntity::Draw() {
  //BG_PALETTE_SUB[0] = RGB5(31, 31, 0);
  ApplyTransformation();

  // Apply animation.
  if (cached_.animation) {
    // make sure the GFX engine is done drawing the previous object
    // while (GFX_STATUS & BIT(14)) {
    //   continue;
    // }
    // while (GFX_STATUS & BIT(27)) {
    //   continue;
    // }
    // BG_PALETTE_SUB[0] = RGB5(0, 31, 31);
    cached_.actor->ApplyAnimation(cached_.animation, cached_.animation_frame);
    // BG_PALETTE_SUB[0] = RGB5(0, 0, 31);
  }

  // Draw the object.
  // BG_PALETTE_SUB[0] = RGB5(31, 0, 31);
  glCallList(cached_.actor->DrawList());
  // BG_PALETTE_SUB[0] = RGB5(0, 0, 31);
}

void DrawableEntity::Update() {
  // Update the animation if one is playing.
  if (current_.animation) {
    current_.animation_frame++;
    // Wrap around to the beginning of the animation.
    if (current_.animation_frame >= current_.animation->length) {
      current_.animation_frame = 0;
    }
  }
}

Vec3 DrawableEntity::GetRealModelCenter() {
  // BG_PALETTE_SUB[0] = RGB5(31, 31, 0);
  // Avoid clobbering the render state for this poll by pushing the current
  // matrix before performing the position test.
  glPushMatrix();
  ApplyTransformation();
  // BG_PALETTE_SUB[0] = RGB5(0, 31, 0);

  // wait for the matrix status to clear, and the geometry engine
  // to not be busy drawing (according to GBATEK, maybe not needed?)
  // Wait for the matrix and geometry engine to not be busy.
  // Todo(Nick) Check if this is needed. GBATEK states it may not be.
  while (GFX_STATUS & BIT(14)) {
    continue;
  }
  while (GFX_STATUS & BIT(27)) {
    continue;
  }

  // Perform a hardware position test on the center of the model.
  PosTest(current_.actor->Center().x.data_, current_.actor->Center().y.data_,
      current_.actor->Center().z.data_);
  Vec3 result;
  result.x.data_ = PosTestXresult();
  result.y.data_ = PosTestYresult();
  result.z.data_ = PosTestZresult();

  glPopMatrix(1);
  return result;
}

void DrawableEntity::SetAnimation(std::string name) {
  current_.animation = current_.actor->GetAnimation(name);
  current_.animation_frame = 0;
}

void DrawableEntity::Init() {
  
}
