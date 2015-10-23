#include "particle_library.h"
#include "particle.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

namespace particle_library {

Particle fire;

void Init(VramAllocator<Texture>* texture_allocator, VramAllocator<TexturePalette>* palette_allocator) {
  fire.texture = texture_allocator->Retrieve("fire.a3i5");
  fire.palette = palette_allocator->Retrieve("fire.a3i5");
  fire.position = Vec3{0_f,0_f,0_f};
  fire.lifespan = 16;
  fire.fade_rate = 1_f / 32_f;
  fire.scale = 2.0_f;
  fire.scale_rate = 0.08_f;
}

}
