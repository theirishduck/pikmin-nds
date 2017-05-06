#include "fire_spout.h"

#include "dsgx.h"
#include "pikmin_game.h"
#include "particle.h"
#include "particle_library.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

namespace fire_spout_ai {

void InitAlways(FireSpoutState& fire_spout) {
  // Initialize our model
  fire_spout.entity->set_actor(fire_spout.game->ActorAllocator()->Retrieve("fire_spout"));

  // Set our initial timer to something appropriate
  fire_spout.flame_timer = (rand() % 128);

  // Setup our static physics properties
  fire_spout.body->collision_group = ATTACK_GROUP;

  fire_spout.detection = fire_spout.entity->engine()->World().AllocateBody(fire_spout.handle).body;
  fire_spout.detection->position = fire_spout.position();
  fire_spout.detection->radius = 10_f;
  fire_spout.detection->height = 5_f;
  fire_spout.detection->is_sensor = true;
  fire_spout.detection->collision_group = DETECT_GROUP;
}

void FlameOn(FireSpoutState& fire_spout) {
  // Spawn in a physics entity for the fire hazard
  fire_spout.flame_sensor = fire_spout.entity->engine()->World().AllocateBody().body;
  fire_spout.flame_sensor->radius = 2.0_f;
  fire_spout.flame_sensor->is_sensor = true;
  fire_spout.flame_sensor->collision_group = FIRE_HAZARD_GROUP;
  fire_spout.flame_sensor->position = fire_spout.position();

  //fire_spout.flame_timer = (rand() % 16) + 112;
  fire_spout.flame_timer = 128;
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

void SpawnFireParticle(FireSpoutState& fire_spout) {
  if ((fire_spout.frames_at_this_node & 0x1) == 0) {
    Particle* fire_particle = SpawnParticle(particle_library::fire);
    fire_particle->position = fire_spout.position();
    fire_particle->position.y += 0.5_f;
    fire_particle->velocity = particle_library::FireSpread();
    fire_particle->velocity.y += 0.5_f;
    fire_particle->acceleration = Vec3{0_f,0.005_f,0_f};
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
  for (int i = 0; i < 16; i++) {
    Particle* smoke = SpawnParticle(particle_library::smoke);
    smoke->position = fire_spout.position();
    smoke->position.y += 0.5_f;
    smoke->velocity = particle_library::RandomSpread() * 0.3_f;
    smoke->velocity.y = 0_f;
  }

  // Clear out all of our collision data, so the pikmin stop attacking us
  fire_spout.entity->engine()->World().FreeBody(fire_spout.detection);
  fire_spout.detection = nullptr;
  fire_spout.entity->body_handle().body->owner = Handle();
}

Edge<FireSpoutState> init[] {
  Edge<FireSpoutState>{Trigger::kAlways, nullptr, InitAlways, 1},
  END_OF_EDGES(FireSpoutState)
};

Edge<FireSpoutState> flame_off[] {
  {Trigger::kAlways, FlameTimerExpired, FlameOn, 2},
  {Trigger::kAlways, OutOfHealth, KillSelf, 3},
  END_OF_EDGES(FireSpoutState)
};

Edge<FireSpoutState> flame_on[] {
  {Trigger::kAlways, FlameTimerExpired, FlameOff, 1},
  {Trigger::kAlways, OutOfHealth, KillSelf, 3},
  {Trigger::kAlways, nullptr, SpawnFireParticle, 2}, // Loopback
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
