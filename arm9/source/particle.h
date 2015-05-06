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
};

void UpdateParticles();
Particle* SpawnParticle(Particle& prototype);
void DrawParticles(Vec3 camera_position, Vec3 target_position);

#endif
