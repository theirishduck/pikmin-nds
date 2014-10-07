#include "red_pikmin.h"

#include <stdio.h>

#include "dsgx.h"
#include "pikmin_dsgx.h"

using entities::RedPikmin;

namespace nt = numeric_types;
using numeric_types::literals::operator"" _f;
using numeric_types::fixed;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

RedPikmin::RedPikmin() {
  Dsgx* pikmin_actor = new Dsgx((u32*)pikmin_dsgx, pikmin_dsgx_size);
  set_actor(pikmin_actor);
  SetAnimation("Armature|Run");
}

RedPikmin::~RedPikmin() {
  delete actor();
}

void RedPikmin::Update(MultipassEngine* engine) {
  set_rotation(0_brad, rotation_ + 90_brad, 0_brad);

  updates_until_new_target_--;

  if (NeedsNewTarget()) {
    ChooseNewTarget();
  }

  Move();

  DrawableEntity::Update(engine);
}

bool RedPikmin::NeedsNewTarget() const {
  return updates_until_new_target_ <= 0;
}

void RedPikmin::ChooseNewTarget() {
  target_.x = fixed::FromInt((rand() & 63) - 32);
  target_.y = 0_f;
  target_.z = fixed::FromInt((rand() & 63) - 32);

  updates_until_new_target_ = (rand() & 127) + 128;

  direction_ = (target_ - position()).Normalize();
  rotation_ = Brads::Raw((direction_.z <= 0_f ? 1 : -1) *
      acosLerp(direction_.x.data_));

  // printf("\nTarget: %.1f, %.1f, %.1f\n", (float)target_.x,
  //     (float)target_.y, (float)target_.z);
}

void RedPikmin::Move() {
  nt::Fixed<s32, 12> distance{(target_ - position()).Length2()};
  bool const target_is_far_enough_away{distance > 5.0_f * 5.0_f};
  if (target_is_far_enough_away and not running_) {
    SetAnimation("Armature|Run");
  }
  if (not target_is_far_enough_away and running_) {
    //SetAnimation("Armature|Idle");
  }
  running_ = target_is_far_enough_away;

  if (running_) {
    set_position(position() + Vec3{direction_.x / 4_f, 0_f, direction_.z / 4_f});
    set_rotation(0_brad, rotation_ + 90_brad, 0_brad);
  }
}

