#include "yellow_pikmin.h"

void YellowPikmin::update(MultipassEngine* engine) {
    setRotation(0,degreesToAngle(rotation_),0);
    rotation_ -= 1;
}