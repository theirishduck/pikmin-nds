#include "Captain.h"

void Captain::update(MultipassEngine* engine) {
    setRotation({0,rotation / 2,0});
    rotation += 1;

    //call the draw function's update
    DrawableEntity::update(engine);
}