#ifndef LEVEL_H
#define LEVEL_H

#include "drawable_entity.h"
#include "vram_allocator.h"

#include "physics/world.h"

// This is a simple container for the current level, and all of its
// data elements and properties.
namespace entities {

class Level : public DrawableEntity {
  public:
    Level(physics::World& world);
};

}  // namespace entities

#endif
