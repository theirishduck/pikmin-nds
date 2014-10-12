#include "yellow_pikmin.h"

using entities::YellowPikmin;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

void YellowPikmin::Update() {
  set_rotation(0_brad, rotation_, 0_brad);
  rotation_ -= 1_brad;
}
