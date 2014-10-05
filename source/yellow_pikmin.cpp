#include "yellow_pikmin.h"

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

void YellowPikmin::Update(MultipassEngine* engine) {
  set_rotation(0_brad, rotation_, 0_brad);
  rotation_ -= 1_brad;
}
