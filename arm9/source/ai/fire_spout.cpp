#include "fire_spout.h"

#include "dsgx.h"
#include "pikmin_game.h"
#include "particle.h"

// Model data
#include "fire_spout_dsgx.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

namespace fire_spout_ai {

Dsgx fire_spout_actor((u32*)fire_spout_dsgx, fire_spout_dsgx_size);

void InitAlways(FireSpoutState& fire_spout) {
  // Initialize our model
  fire_spout.entity->set_actor(&fire_spout_actor);
  fire_spout_actor.ApplyTextures(fire_spout.game->TextureAllocator(), fire_spout.game->TexturePaletteAllocator());

  // Set our initial timer to something appropriate
  fire_spout.flame_timer = (rand() % 128);

  // Setup our static physics properties
  fire_spout.entity->body()->collision_group = ATTACK_GROUP;
  fire_spout.entity->body()->owner = &fire_spout.health;

  fire_spout.detection = fire_spout.entity->engine()->World().AllocateBody(&fire_spout);
  fire_spout.detection->position = fire_spout.entity->body()->position;
  fire_spout.detection->radius = 10_f;
  fire_spout.detection->height = 5_f;
  fire_spout.detection->is_sensor = true;
  fire_spout.detection->collision_group = DETECT_GROUP;
  fire_spout.detection->owner = fire_spout.entity->body();
}

void FlameOn(FireSpoutState& fire_spout) {
  // Spawn in a physics entity for the fire hazard
  fire_spout.flame_sensor = fire_spout.entity->engine()->World().AllocateBody(&fire_spout);
  fire_spout.flame_sensor->radius = 2.0_f;
  fire_spout.flame_sensor->is_sensor = true;
  fire_spout.flame_sensor->collision_group = FIRE_HAZARD_GROUP;
  fire_spout.flame_sensor->position = fire_spout.entity->body()->position;

  fire_spout.flame_timer = (rand() % 16) + 112;
}

void FlameOff(FireSpoutState& fire_spout) {
  fire_spout.entity->engine()->World().FreeBody(fire_spout.flame_sensor);
  fire_spout.flame_sensor = nullptr;

  fire_spout.flame_timer = (rand() % 16) + 112;
}

bool FlameTimerExpired(const FireSpoutState& fire_spout) {
  if (fire_spout.frames_at_this_node > fire_spout.flame_timer) {
    return true;
  }
  return false;
}

//returns a random vector from -1 to 1 in all directions
Vec3 FireSpread() {
  return Vec3{
    fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12)),
    fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12)),
    fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12))
  };
}

void SpawnFireParticle(FireSpoutState& fire_spout) {
  if ((fire_spout.frames_at_this_node & 0x1) == 0) {
    Particle fire_particle;
    fire_particle.texture = fire_spout.game->TextureAllocator()->Retrieve("fire.a3i5");
    fire_particle.palette = fire_spout.game->TexturePaletteAllocator()->Retrieve("fire.a3i5");
    fire_particle.position = fire_spout.entity->body()->position;
    fire_particle.position.y += 0.5_f;
    fire_particle.lifespan = 16;
    fire_particle.fade_rate = 1_f / 32_f;
    fire_particle.scale = 2.0_f;
    fire_particle.scale_rate = 0.08_f;

    Particle* new_particle = SpawnParticle(fire_particle);
    new_particle->velocity = FireSpread() * 0.06_f;
    new_particle->velocity.y += 0.5_f;
    new_particle->acceleration = Vec3{0_f,0.005_f,0_f};
  }
}

bool OutOfHealth(const FireSpoutState& fire_spout) {
  return fire_spout.health <= 0;
}

void KillSelf(FireSpoutState& fire_spout) {
  // If we presently have our hazard bubble up, kill it.
  if (fire_spout.flame_sensor) {
    FlameOff(fire_spout);
  }

  // TODO: Spawn a ring of gas particles to indicate death

  // Clear out all of our collision data, so the pikmin stop attacking us
  fire_spout.entity->engine()->World().FreeBody(fire_spout.detection);
  fire_spout.detection = nullptr;
  fire_spout.entity->body()->owner = nullptr;
}

Edge<FireSpoutState> init[] {
  Edge<FireSpoutState>{kAlways, nullptr, InitAlways, 1},
  END_OF_EDGES(FireSpoutState)
};

Edge<FireSpoutState> flame_off[] {
  {kAlways, FlameTimerExpired, FlameOn, 2},
  {kAlways, OutOfHealth, KillSelf, 3},
  END_OF_EDGES(FireSpoutState)
};

Edge<FireSpoutState> flame_on[] {
  {kAlways, FlameTimerExpired, FlameOff, 1},
  {kAlways, OutOfHealth, KillSelf, 3},
  {kAlways, nullptr, SpawnFireParticle, 2}, // Loopback
  END_OF_EDGES(FireSpoutState)
};

Edge<FireSpoutState> dead[] {
  END_OF_EDGES(FireSpoutState)
};

Node<FireSpoutState> node_list[] {
  {"Init", true, init},
  {"FlameOff", true, flame_off},
  {"FlameOn", true, flame_on},
  {"Dead", true, dead},
};

StateMachine<FireSpoutState> machine(node_list);

}  // namespace fire_spout_ai
