#ifndef LEVEL_H
#define LEVEL_H

#include "drawable_entity.h"
#include "vram_allocator.h"

// This is a simple container for the current level, and all of its
// data elements and properties.
namespace entities {

class Level : public DrawableEntity {
  public:
    Level(VramAllocator* texture_allocator, VramAllocator* palette_allocator);
    ~Level();

    void Init() override;
};

}  // namespace entities

#endif
