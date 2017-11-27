#include "drawable.h"

#include <cstdio>

#include <nds/arm9/postest.h>

namespace nt = numeric_types;

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::fixed;
using numeric_types::Brads;

Drawable::Drawable() {
  //zero out the cached matrix to initialize it
  for (int i = 1; i < 13; i++) {
    cached_matrix_[i] = 0;
  }

  //the first byte needs to be the geometry command for a
  //MATRIX_MUL4x3
  cached_matrix_[0] = 0x19;
  current_.scale = 1.0_f;
}

Vec3 Drawable::position() const {
  return current_.position;
}

void Drawable::set_position(Vec3 pos) {
  current_.position = pos;
}

Rotation Drawable::rotation() const {
  return current_.rotation;
}

void Drawable::set_rotation(nt::Brads x, nt::Brads y, nt::Brads z) {
  current_.rotation.x = x;
  current_.rotation.y = y;
  current_.rotation.z = z;
}

void Drawable::set_rotation(Rotation rotation) {
  current_.rotation = rotation;
}

fixed Drawable::scale() const {
  return current_.scale;
}

void Drawable::set_scale(fixed new_scale) {
  current_.scale = new_scale;
}

DrawState& Drawable::GetCachedState() {
  return cached_;
}

void Drawable::SetCache() {
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

void Drawable::set_actor(Dsgx* actor) {
  current_.actor = actor;
  current_.current_mesh = actor->DefaultMesh();
}

Dsgx* Drawable::actor() {
  return current_.actor;
}

void Drawable::set_mesh(const char* mesh_name) {
  current_.current_mesh = current_.actor->MeshByName(mesh_name);
}

Mesh* Drawable::mesh() {
  return current_.current_mesh;
}

void Drawable::ApplyTransformation() {
  if (cached_.rotation.x.data_ or cached_.rotation.z.data_ or cached_.scale != 1.0_f) {
    // sub-optimal case. This is correct, but slow; I don't know how to
    // improve arbitrary rotation yet. -Nick
    glTranslatef32(cached_.position.x.data_, cached_.position.y.data_,
        cached_.position.z.data_);

    glScalef32(cached_.scale.data_, cached_.scale.data_, cached_.scale.data_);

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

void Drawable::Draw() {
  if (cached_.actor == nullptr or cached_.current_mesh == nullptr) {
    return;
  }
  ApplyTransformation();

  // Apply animation.
  if (cached_.animation) {
    cached_.actor->ApplyAnimation(cached_.animation, cached_.animation_frame, cached_.current_mesh);
  }

  // Draw the object using display lists.
  glCallList(cached_.current_mesh->model_data);

}

void Drawable::Update() {
  // Update the animation if one is playing.
  if (current_.animation) {
    current_.animation_frame++;
    // Wrap around to the beginning of the animation.
    if (current_.animation_frame >= current_.animation->frame_length) {
      current_.animation_frame = 0;
    }
  }
}

bool Drawable::InsideViewFrustrum() {
  // Determine if this object is in the view frustrum using a BOX_TEST
  glPushMatrix();
  ApplyTransformation();
  auto radius = cached_.current_mesh->bounding_radius;
  glScalef32(radius.data_, radius.data_, radius.data_);

  bool result = BoxTest(
      (-1_f).data_,
      (-1_f).data_,
      (-1_f).data_,
      (2_f).data_,
      (2_f).data_,
      (2_f).data_);

  glPopMatrix(1);

  return result;
}

numeric_types::fixed Drawable::GetRealModelZ() {
  // Avoid clobbering the render state for this poll by pushing the current
  // matrix before performing the position test.
  glPushMatrix();
  ApplyTransformation();

  // Perform a hardware position test on the center of the model.
  Vec3& center = cached_.current_mesh->bounding_center;
  PosTest(center.x.data_, center.y.data_, center.z.data_);
  numeric_types::fixed result;
  result.data_ = PosTestZresult();

  glPopMatrix(1);
  return result;
}

void Drawable::SetAnimation(std::string name) {
  current_.animation = current_.actor->GetAnimation(name, current_.current_mesh);
  current_.animation_frame = 0;
}

void Drawable::RotateToFace(Brads target_angle, Brads rate) {
  auto delta = target_angle - current_.rotation.y;

  // clamp the delta so that it is within -180, 180
  while (delta > 180_brad) {
    delta -= 360_brad;
  }
  while (delta < -180_brad) {
    delta += 360_brad;
  }

  // if the delta is greater than the rate, limit it for slow turning
  if (delta > rate) {
    delta = rate;
  }
  if (delta < -rate) {
    delta = -rate;
  }

  current_.rotation.y += delta;
}

Brads Drawable::AngleTo(const Drawable* destination) {
  auto difference = Vec2{destination->position().x, destination->position().z} -
      Vec2{position().x, position().z};
  if (difference.Length2() > 0_f) {
    difference = difference.Normalize();
    if (difference.y <= 0_f) {
      return Brads::Raw(acosLerp(difference.x.data_));
    }
    return Brads::Raw(-acosLerp(difference.x.data_));
  }
  return 0_brad;
}

void Drawable::RotateToFace(const Drawable* destination, Brads rate) {
  RotateToFace(AngleTo(destination), rate);
}

u32 Drawable::CurrentFrame() {
  return current_.animation_frame;
}