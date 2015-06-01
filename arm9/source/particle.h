#ifndef PARTICLE_H
#define PARTICLE_H

#include "vector.h"
#include "vram_allocator.h"

struct Particle {
  Vec3 position;
  Vec3 velocity;
  Vec3 acceleration;

  u16 lifespan;
  u16 age;

  bool active;
  Texture texture;
  TexturePalette palette;

  numeric_types::fixed alpha{numeric_types::fixed::FromInt(1)};
  numeric_types::fixed fade_rate;
};

void UpdateParticles();
Particle* SpawnParticle(Particle& prototype);
void DrawParticles(Vec3 camera_position, Vec3 target_position);

#endif
