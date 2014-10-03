#include "yellow_pikmin.h"

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

void YellowPikmin::update(MultipassEngine* engine) {
    setRotation(0_brad,rotation_,0_brad);
    rotation_ -= 1_brad;
}