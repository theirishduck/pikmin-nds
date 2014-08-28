#include "Captain.h"

//model data
#include "DSGX.h"
#include "olimar_dsgx.h"
#include "olimar_low_poly_dsgx.h"
#include "test_dsgx.h"

Captain::Captain() {
    //todo: not this. This creates a new DSGX per instance of the object which, while not
    //huge, is still a processing load that should be avoided.
    DSGX* olimar_actor = new DSGX((u32*)olimar_low_poly_dsgx, olimar_low_poly_dsgx_size);
    setActor(olimar_actor);
    setAnimation("Armature|Idle1");
}

void Captain::update(MultipassEngine* engine) {
    if (running) {
        if (!(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))) {
            running = false;
            setAnimation("Armature|Idle1");
        }
    } else {
        if (keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
            running = true;
            setAnimation("Armature|Run");
        }
    }

    int dpad_angle = engine->dPadDirection();
    int delta = dpad_angle - current_angle;
    if (delta >= 180)  {delta -= 360;}
    if (delta < -180)  {delta += 360;}

    current_angle += delta >> 3;
    if (current_angle >= 360) {current_angle -= 360;}
    if (current_angle <    0) {current_angle += 360;}


    setRotation({0,current_angle + 90,0});

    //call the draw function's update
    DrawableEntity::update(engine);
}