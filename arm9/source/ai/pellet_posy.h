#ifndef AI_POSY_H
#define AI_POSY_H

#include "state_machine.h"
#include "drawable_entity.h"

namespace posy_ai {

struct PosyState : ObjectState {
  int health{50};
};

}  // namespace posy_ai

#endif
