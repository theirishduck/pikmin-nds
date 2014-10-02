#include "Captain.h"

#include <nds/arm9/input.h>

#include "multipass_engine.h"

//model data
#include "dsgx.h"
#include "olimar_dsgx.h"
#include "olimar_low_poly_dsgx.h"
#include "test_dsgx.h"

using numeric_types::literals::operator"" _f;

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
    if (running_) {
        if (!(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))) {
            running_ = false;
            setAnimation("Armature|Idle1");
        }
    } else {
        if (keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
            running_ = true;
            setAnimation("Armature|Run");
        }
    }

    if (running_) {
        int dpad_angle = angleToDegrees(engine->cameraAngle()) + (engine->dPadDirection() - 90);
        int delta = dpad_angle - current_angle_;

        //translate delta to a good relative range
        while (delta >= 180)  {delta -= 360;}
        while (delta < -180)  {delta += 360;}
        //clamp it to limit the maximum turning angle per frame
        if (delta >  11) {delta =  11;}
        if (delta < -11) {delta = -11;}

        current_angle_ += delta;
        //now, make sure current_angle stays in a sane range and doesn't overflow
        if (current_angle_ >= 360) {current_angle_ -= 360;}
        if (current_angle_ <    0) {current_angle_ += 360;}

        setRotation(0,degreesToAngle(current_angle_ + 90),0);

        //finally, movement! Based on our angle, apply a velocity in that direction
        //(Note: This is kind of backwards? Maybe we should be working with a direction vector)
        Vec3 movement;
        movement.x.data_ = cosLerp(degreesToAngle(current_angle_));
        movement.z.data_ = -sinLerp(degreesToAngle(current_angle_));
        setPosition(position() + 
            movement * 0.2_f);
    }

    //call the draw function's update
    DrawableEntity::update(engine);
}
