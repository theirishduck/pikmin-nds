#ifndef PARTICLE_LIBRARY_H
#define PARTICLE_LIBRARY_H

#include "vram_allocator.h"
struct Particle;

namespace particle_library {

void Init(VramAllocator<Texture>* texture_allocator, VramAllocator<TexturePalette>* palette_allocator);

extern Particle fire;

}  // namespace particle_library

#endif
