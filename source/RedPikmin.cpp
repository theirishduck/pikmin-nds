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
    setRotation(0,rotation + degreesToAngle(90),0);

    //let's do something fun
    if (nextAnim <= 0) {
        if (running) {
                setAnimation("Armature|Idle");    
                running = false;
                nextAnim = 30 + (rand() & 0x1F);
        } else {
            setAnimation("Armature|Run");
            rotation = rand() & 0xFFFF;
            running = true;
            nextAnim = 60 + (rand() & 0x1F);
        }
    }

    nextAnim--;

    if (running) {
        //move the pikmin! scary
        gx::Fixed<s32,12> vx; vx.data = cosLerp(rotation);
        gx::Fixed<s32,12> vz; vz.data = sinLerp(rotation);

        setPosition(position() + Vec3{vx / 16, 0, vz / -16});
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