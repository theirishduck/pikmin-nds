#include "RedPikmin.h"
#include "DSGX.h"
#include "pikmin_dsgx.h"

RedPikmin::RedPikmin() {
    DSGX* pikmin_actor = new DSGX((u32*)pikmin_dsgx, pikmin_dsgx_size);
    setActor(pikmin_actor);
    setAnimation("Armature|Run");
}

RedPikmin::~RedPikmin() {
    delete getActor();
}

void RedPikmin::update(MultipassEngine* engine) {
    setRotation(0,degreesToAngle(rotation),0);
    rotation += 1;

    //call the draw function's update
    DrawableEntity::update(engine);
}