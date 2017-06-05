#ifndef PARTICLE_LIBRARY_H
#define PARTICLE_LIBRARY_H

#include "numeric_types.h"
#include "vram_allocator.h"

struct Particle;

namespace particle_library {

void Init(VramAllocator<Texture>* texture_allocator, VramAllocator<TexturePalette>* palette_allocator);

extern Particle dirt_cloud;
extern Particle fire;
extern Particle smoke;
extern Particle piki_star;
extern Particle rock;

Vec3 DirtCloudSpread();
Vec3 FireSpread();
Vec3 RandomSpread();
Vec3 RockSpread();


void SpreadPikiStar(Particle* particle);

}  // namespace particle_library

#endif
