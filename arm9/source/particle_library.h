#ifndef PARTICLE_LIBRARY_H
#define PARTICLE_LIBRARY_H

#include "vram_allocator.h"
#include "numeric_types.h"
struct Particle;

namespace particle_library {

void Init(VramAllocator<Texture>* texture_allocator, VramAllocator<TexturePalette>* palette_allocator);

extern Particle dirt_rock;
extern Particle fire;
extern Particle smoke;
extern Particle piki_star;

Vec3 RandomSpread();
Vec3 FireSpread();
Vec3 DirtSpread();

void SpreadPikiStar(Particle* particle);

}  // namespace particle_library

#endif
