#include "RedPikmin.h"

void RedPikmin::update(MultipassEngine* engine) {
    setRotation(0,degreesToAngle(rotation),0);
    rotation += 1;
}