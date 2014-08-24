#include "YellowPikmin.h"

void YellowPikmin::update(MultipassEngine* engine) {
    setRotation({0,rotation,0});
    rotation -= 1;
}