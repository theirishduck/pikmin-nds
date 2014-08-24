#include "Captain.h"

void Captain::update(MultipassEngine* engine) {
    setRotation({180,rotation,0});
    rotation += 1;
}