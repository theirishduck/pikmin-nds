#include "Captain.h"

#include <nds/arm9/input.h>

#include "multipass_engine.h"

//model data
#include "dsgx.h"
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

Captain::~Captain() {
    //todo: not this. Use a manager instead.
    delete getActor();
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

    if (running) {
        int dpad_angle = angleToDegrees(engine->cameraAngle()) + (engine->dPadDirection() - 90);
        int delta = dpad_angle - current_angle;

        //translate delta to a good relative range
        while (delta >= 180)  {delta -= 360;}
        while (delta < -180)  {delta += 360;}
        //clamp it to limit the maximum turning angle per frame
        if (delta >  11) {delta =  11;}
        if (delta < -11) {delta = -11;}

        current_angle += delta;
        //now, make sure current_angle stays in a sane range and doesn't overflow
        if (current_angle >= 360) {current_angle -= 360;}
        if (current_angle <    0) {current_angle += 360;}

        setRotation(0,degreesToAngle(current_angle + 90),0);

        //finally, movement! Based on our angle, apply a velocity in that direction
        //(Note: This is kind of backwards? Maybe we should be working with a direction vector)
        Vec3 movement;
        movement.x.data = cosLerp(degreesToAngle(current_angle));
        movement.z.data = -sinLerp(degreesToAngle(current_angle));
        setPosition(position() + movement * 0.2);
    }

    //call the draw function's update
    DrawableEntity::update(engine);
}
