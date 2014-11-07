#include "pikmin.h"

#include <stdio.h>

#include "dsgx.h"
#include "red_pikmin_dsgx.h"
#include "yellow_pikmin_dsgx.h"
#include "blue_pikmin_dsgx.h"
#include "debug.h"

using entities::Pikmin;
using entities::PikminType;

namespace nt = numeric_types;
using numeric_types::literals::operator"" _f;
using numeric_types::fixed;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

Pikmin::Pikmin(PikminType type) {
  // initialize all the pikmin actors
  // Todo(Nick): Replace this with the asset loader logic; this is very
  // inefficient right now.
  red_pikmin_actor = new Dsgx((u32*)red_pikmin_dsgx, red_pikmin_dsgx_size);
  yellow_pikmin_actor = new Dsgx((u32*)yellow_pikmin_dsgx, yellow_pikmin_dsgx_size);
  blue_pikmin_actor = new Dsgx((u32*)blue_pikmin_dsgx, blue_pikmin_dsgx_size);

  SetPikminType(type);
  SetAnimation("Armature|Idle");
}

void Pikmin::SetPikminType(PikminType type) {
  switch (type) {
    case PikminType::kRedPikmin:
      set_actor(red_pikmin_actor);
      break;
    case PikminType::kYellowPikmin:
      set_actor(yellow_pikmin_actor);
      break;
    case PikminType::kBluePikmin:
      set_actor(blue_pikmin_actor);
      break;
  }
}

Pikmin::~Pikmin() {
  delete red_pikmin_actor;
  delete yellow_pikmin_actor;
  delete blue_pikmin_actor;
}

void Pikmin::Init() {
  body_ = engine()->World().AllocateBody(this, 4_f, 1_f);
  body_->position = position();
  body_->collides_with_bodies = 1;
  body_->is_movable = 1;
  body_->is_pikmin = 1;
}

void Pikmin::Update() {
  set_rotation(0_brad, rotation_ + 90_brad, 0_brad);

  updates_until_new_target_--;

  if (NeedsNewTarget()) {
    ChooseNewTarget();
  }

  Move();

  DrawableEntity::Update();

  debug::DrawCircle(body_->position, body_->radius, RGB5(31,31,15));
}

bool Pikmin::NeedsNewTarget() const {
  return false;
  return updates_until_new_target_ <= 0;
}

void Pikmin::ChooseNewTarget() {
  target_.x = fixed::FromInt((rand() & 63) - 32);
  target_.y = 0_f;
  target_.z = fixed::FromInt((rand() & 63) - 32);

  updates_until_new_target_ = (rand() & 127) + 256;

  direction_ = (target_ - position()).Normalize();
  rotation_ = Brads::Raw((direction_.z <= 0_f ? 1 : -1) *
      acosLerp(direction_.x.data_));

  // printf("\nTarget: %.1f, %.1f, %.1f\n", (float)target_.x,
  //     (float)target_.y, (float)target_.z);
}

void Pikmin::Move() {
  nt::Fixed<s32, 12> distance{(target_ - position()).Length2()};
  bool const target_is_far_enough_away{distance > 5.0_f * 5.0_f};
  if (target_is_far_enough_away and not running_) {
    SetAnimation("Armature|Run");
  }
  if (not target_is_far_enough_away and running_) {
    SetAnimation("Armature|Idle");
  }
  running_ = target_is_far_enough_away;

  if (running_) {
    body_->velocity.x = direction_.x * 0.25_f;
    body_->velocity.y = 0_f;
    body_->velocity.z = direction_.z * 0.25_f;

    set_rotation(0_brad, rotation_ + 90_brad, 0_brad);
  } else {
    body_->velocity = Vec3{0_f, 0_f, 0_f};
  }
  set_position(body_->position);
}

