#include "RedPikmin.h"
#include "DSGX.h"
#include "pikmin_dsgx.h"

#include <stdio.h>


RedPikmin::RedPikmin() {
    DSGX* pikmin_actor = new DSGX((u32*)pikmin_dsgx, pikmin_dsgx_size);
    setActor(pikmin_actor);
    setAnimation("Armature|Run");
}

RedPikmin::~RedPikmin() {
    delete getActor();
}

void RedPikmin::update(MultipassEngine* engine) {
    setRotation(0,rotation + degreesToAngle(90),0);

    nextTarget--;
    //let's do something fun
    if (nextTarget <= 0) {
        target.x = (rand() % 64) - 32;
        target.y = 0;
        target.z = (rand() % 64) - 32;

        nextTarget = (rand() % 128) + 128;
    }

    printf("\nTarget: %.1f, %.1f, %.1f\n", (float)target.x, (float)target.y, (float)target.z);

    //figure out if we need to run toward our target
    gx::Fixed<s32,12> distance = (target - position()).length();
    Vec3 direction = (target - position()).normalize();
    if (distance > 5.0f) {
        if (!running) {
            setAnimation("Armature|Run");
        }
        running = true;
        //figure out the run rotation from our direction vector
        if (direction.z <= 0) {
            rotation = acosLerp(direction.x.data);
        } else {
            rotation = -acosLerp(direction.x.data);
        }
    } else {
        running = false;
    }

    if (running) {
        //move the pikmin! scary
        setPosition(position() + Vec3{direction.x / 4, 0, direction.z / 4});
        setRotation(0, rotation + degreesToAngle(90), 0);
        
    } else {
        setAnimation("Armature|Idle");
    }

    /*
    if (keysHeld() & KEY_RIGHT)
        rotation += 1;  
    if (keysHeld() & KEY_LEFT)
        rotation -= 1;  
    */

    //call the draw function's update
    DrawableEntity::update(engine);
}