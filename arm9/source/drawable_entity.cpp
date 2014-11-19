#include "drawable_entity.h"

#include <cstdio>

#include <nds/arm9/postest.h>

#include "multipass_engine.h"

namespace nt = numeric_types;

DrawableEntity::DrawableEntity() {
  //zero out the cached matrix to initialize it
  for (int i = 1; i < 13; i++) {
    cached_matrix_[i] = 0;
  }

  //the first byte needs to be the geometry command for a
  //MATRIX_MUL4x3
  cached_matrix_[0] = 0x19;
}

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

  //return;

  //calculate (once) the transformation matrix for this object
  //this is lifted largely from ndslib
  int sine = sinLerp(cached_.rotation.y.data_);
  int cosine = cosLerp(cached_.rotation.y.data_);

  cached_matrix_[1] = cosine;
  //cached_matrix_[2] = 0;
  cached_matrix_[3] = -sine;

  //cached_matrix_[4] = 0;
  cached_matrix_[5] = inttof32(1);
  //cached_matrix_[6] = 0;
  
  cached_matrix_[7] = sine;
  //cached_matrix_[8] = 0;
  cached_matrix_[9] = cosine;
  
  cached_matrix_[10]  = cached_.position.x.data_;
  cached_matrix_[11] = cached_.position.y.data_;
  cached_matrix_[12] = cached_.position.z.data_;
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
  if (cached_.rotation.x.data_ or cached_.rotation.z.data_) {
    // sub-optimal case. This is correct, but slow; I don't know how to
    // improve arbitrary rotation yet. -Nick
    glTranslatef32(cached_.position.x.data_, cached_.position.y.data_,
        cached_.position.z.data_);

    glRotateYi(cached_.rotation.y.data_);
    glRotateXi(cached_.rotation.x.data_);
    glRotateZi(cached_.rotation.z.data_);
  } else {
    // optimized case, for a translation and a rotation about only the Y-axis.
    // This uses a pre-calculated matrix.

    // This ends up being slightly faster than using DMA transfers for some reason on real hardware, by about
    // 5k hardware cycles for 100 objects drawn. It also dodges needing to worry about the cache, which is
    // a plus.

    MATRIX_MULT4x3 = cached_matrix_[1];
    MATRIX_MULT4x3 = cached_matrix_[2];
    MATRIX_MULT4x3 = cached_matrix_[3];

    MATRIX_MULT4x3 = cached_matrix_[4];
    MATRIX_MULT4x3 = cached_matrix_[5];
    MATRIX_MULT4x3 = cached_matrix_[6];

    MATRIX_MULT4x3 = cached_matrix_[7];
    MATRIX_MULT4x3 = cached_matrix_[8];
    MATRIX_MULT4x3 = cached_matrix_[9];

    MATRIX_MULT4x3 = cached_matrix_[10];
    MATRIX_MULT4x3 = cached_matrix_[11];
    MATRIX_MULT4x3 = cached_matrix_[12];
  }
}

void DrawableEntity::Draw() {
  ApplyTransformation();

  // Apply animation.
  if (cached_.animation) {
    cached_.actor->ApplyAnimation(cached_.animation, cached_.animation_frame);
  }

  // Draw the object using display lists.
  glCallList(cached_.actor->DrawList());
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

  //set the current position to our body's physics position
  set_position(body_->position);
}

numeric_types::fixed DrawableEntity::GetRealModelZ() {
  // Avoid clobbering the render state for this poll by pushing the current
  // matrix before performing the position test.
  glPushMatrix();
  ApplyTransformation();

  // Perform a hardware position test on the center of the model.
  Vec3& center = current_.actor->Center();
  PosTest(center.x.data_, center.y.data_, center.z.data_);
  numeric_types::fixed result;
  result.data_ = PosTestZresult();

  glPopMatrix(1);
  return result;
}

void DrawableEntity::SetAnimation(std::string name) {
  current_.animation = current_.actor->GetAnimation(name);
  current_.animation_frame = 0;
}

void DrawableEntity::Init() {
  body_ = engine()->World().AllocateBody(this);
}

physics::Body* DrawableEntity::body() {
  return body_;
}