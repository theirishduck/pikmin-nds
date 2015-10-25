#include "particle_library.h"
#include "particle.h"
#include "project_settings.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

namespace particle_library {

Particle fire;
Particle dirt_rock;
Particle smoke;

void Init(VramAllocator<Texture>* texture_allocator, VramAllocator<TexturePalette>* palette_allocator) {
  fire.texture = texture_allocator->Retrieve("fire.a3i5");
  fire.palette = palette_allocator->Retrieve("fire.a3i5");
  // Perhaps we should be reading in the width/height from the image on disk?
  fire.texture.format_width = TEXTURE_SIZE_32;
  fire.texture.format_height = TEXTURE_SIZE_32;
  fire.position = Vec3{0_f,0_f,0_f};
  fire.lifespan = 16;
  fire.fade_rate = 1_f / 32_f;
  fire.scale = 2.0_f;
  fire.scale_rate = 0.08_f;

  dirt_rock.texture = texture_allocator->Retrieve("rock.t2bpp");
  dirt_rock.palette = palette_allocator->Retrieve("rock.t2bpp");
  // Perhaps we should be reading in the width/height from the image on disk?
  dirt_rock.texture.format_width = TEXTURE_SIZE_16;
  dirt_rock.texture.format_height = TEXTURE_SIZE_16;
  dirt_rock.position = Vec3{0_f,0_f,0_f};
  dirt_rock.lifespan = 16;
  dirt_rock.fade_rate = 1_f / 32_f;
  dirt_rock.scale = 0.5_f;
  dirt_rock.scale_rate = 0.02_f;
  dirt_rock.velocity = Vec3{0_f,1_f,0_f};
  dirt_rock.acceleration = Vec3{0_f,-GRAVITY_CONSTANT,0_f};

  smoke.texture = texture_allocator->Retrieve("smoke1.a5i3");
  smoke.palette = palette_allocator->Retrieve("smoke1.a5i3");
  smoke.texture.format_width = TEXTURE_SIZE_32;
  smoke.texture.format_height = TEXTURE_SIZE_32;
  smoke.position = Vec3{0_f,0_f,0_f};
  smoke.lifespan = 16;
  smoke.fade_rate = 1_f / 16_f;
  smoke.scale = 2.0_f;
  smoke.scale_rate = 0.08_f;
  smoke.color_a = RGB15(28,20,0);
  smoke.color_b = RGB15(20,4,0);
  smoke.color_change_rate = 8;
}

// Utility functions for setting particle properties and variance

//returns a random vector from -1 to 1 in all directions
Vec3 RandomSpread() {
  return Vec3{
    fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12)),
    fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12)),
    fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12))
  };
}

Vec3 FireSpread() {
  return RandomSpread() * 0.06_f;
}

Vec3 DirtSpread() {
  auto vel = RandomSpread();
  vel.x *= 0.4_f;
  vel.y *= 0.06_f;
  vel.z *= 0.4_f;
  return vel;
}

}
