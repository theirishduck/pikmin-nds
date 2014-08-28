#include "Captain.h"

void Captain::update(MultipassEngine* engine) {
    setRotation({0,engine->dPadDirection() + 90,0});    

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

    //call the draw function's update
    DrawableEntity::update(engine);
}