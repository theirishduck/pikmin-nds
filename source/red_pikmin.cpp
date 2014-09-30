#include "red_pikmin.h"

#include <stdio.h>

#include "dsgx.h"
#include "pikmin_dsgx.h"

namespace nt = numeric_types;

RedPikmin::RedPikmin() {
    DSGX* pikmin_actor = new DSGX((u32*)pikmin_dsgx, pikmin_dsgx_size);
    setActor(pikmin_actor);
    setAnimation("Armature|Run");
}

RedPikmin::~RedPikmin() {
    delete getActor();
}

void RedPikmin::update(MultipassEngine* engine) {
    setRotation(0, rotation_ + degreesToAngle(90), 0);

    updates_until_new_target_--;

    if (NeedsNewTarget()) {
        ChooseNewTarget();
    }

    Move();

    DrawableEntity::update(engine);
}

bool RedPikmin::NeedsNewTarget() const {
    return updates_until_new_target_ <= 0;
}

void RedPikmin::ChooseNewTarget() {
    target_.x = rand() % 64 - 32;
    target_.y = 0;
    target_.z = rand() % 64 - 32;

    updates_until_new_target_ = rand() % 128 + 128;

    direction_ = (target_ - position()).normalize();
    rotation_ = (direction_.z <= 0 ? 1 : -1) * acosLerp(direction_.x.data);

    // printf("\nTarget: %.1f, %.1f, %.1f\n", (float)target_.x,
    //     (float)target_.y, (float)target_.z);
}

void RedPikmin::Move() {
    nt::Fixed<s32, 12> distance{(target_ - position()).length()};
    bool const target_is_far_enough_away{distance > 5.0f};
    if (target_is_far_enough_away and not running_) {
        setAnimation("Armature|Run");
    }
    running_ = target_is_far_enough_away;

    if (running_) {
        setPosition(position() + Vec3{direction_.x / 4, 0, direction_.z / 4});
        setRotation(0, rotation_ + degreesToAngle(90), 0);
    } else {
        // setAnimation("Armature|Idle");
    }

    /*
    if (keysHeld() & KEY_RIGHT) {
        rotation_ += 1;
    }
    if (keysHeld() & KEY_LEFT) {
        rotation_ -= 1;
    }
    //*/
}
