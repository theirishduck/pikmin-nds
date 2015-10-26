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

  numeric_types::fixed scale{numeric_types::fixed::FromInt(1)};
  numeric_types::fixed scale_rate;

  numeric_types::Brads rotation{numeric_types::Brads::Raw(degreesToAngle(0))};
  numeric_types::Brads rotation_rate;

  u16 color_a{RGB15(31, 31, 31)};
  u16 color_b{RGB15(31, 31, 31)};
  u8 color_weight = 31;
  u8 color_change_rate = 0;

  // Normal color, is overriden by the above if color_change_rate is set
  u16 color{RGB15(31, 31, 31)};
};

void UpdateParticles();
Particle* SpawnParticle(Particle& prototype);
void DrawParticles(Vec3 camera_position, Vec3 target_position);
int ActiveParticles();

#endif
