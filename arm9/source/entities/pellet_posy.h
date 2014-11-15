#ifndef PELLET_POSY_H
#define PELLET_POSY_H

#include "drawable_entity.h"
#include "vram_allocator.h"

class MultipassEngine;

namespace physics {
class Body;
}  // namespace physics

namespace entities {

class PelletPosy : public DrawableEntity {
  public:
    PelletPosy(VramAllocator& texture_allocator);
    ~PelletPosy();
};

}  // namespace entities

#endif // PELLET_POSY_H