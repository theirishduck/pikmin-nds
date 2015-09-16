#include "pellet_posy.h"

#include "dsgx.h"
#include "pikmin_game.h"

// Model data
#include "pellet_posy_dsgx.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

namespace posy_ai {

Dsgx posy_actor((u32*)pellet_posy_dsgx, pellet_posy_dsgx_size);

void InitAlways(PosyState& posy) {
  posy.entity->set_actor(&posy_actor);
  posy_actor.ApplyTextures(posy.game->TextureAllocator(), posy.game->TexturePaletteAllocator());

  posy.detection = posy.entity->engine()->World().AllocateBody(&posy);
  posy.detection->position = posy.entity->body()->position;
  posy.detection->radius = 10_f;
  posy.detection->height = 5_f;
  posy.detection->is_sensor = true;
  posy.detection->collision_group = DETECT_GROUP;
  posy.detection->owner = posy.entity->body();

  posy.entity->body()->collision_group = ATTACK_GROUP;
  posy.entity->body()->owner = &posy.health;
}

bool ZeroHealth(const PosyState& posy) {
  return posy.health <= 0;
}

bool TookDamage(const PosyState& posy) {
  return posy.health < posy.old_health;
}

void StoreCurrentHealth(PosyState& posy) {
  posy.old_health = posy.health;
}

void GoodbyeCruelWorld(PosyState& posy) {
  posy.entity->engine()->World().FreeBody(posy.detection);
  posy.dead = true;
}

namespace PosyNode {
enum PosyNode {
  kInit = 0,
  kIdle,
  kHit,
  kDeath,
};
}

Edge<PosyState> edge_list[] {
  // Init
  Edge<PosyState>{kAlways, nullptr, InitAlways, PosyNode::kIdle},

  // Idle
  {kAlways, ZeroHealth, nullptr, PosyNode::kDeath},
  {kAlways, TookDamage, nullptr, PosyNode::kHit},
  {kAlways, nullptr, nullptr, PosyNode::kIdle},  // Loopback

  // Hit
  {kAlways, ZeroHealth, nullptr, PosyNode::kDeath},
  {kLastFrame, nullptr, StoreCurrentHealth, PosyNode::kIdle},

  // Death
  {kLastFrame, nullptr, GoodbyeCruelWorld, PosyNode::kDeath},
};

Node node_list[] {
  {"Init", true, 0, 0},
  {"Idle", true, 1, 3, "Armature|Idle", 30},
  {"Hit", true, 4, 5, "Armature|Hit", 15},
  {"Death", true, 6, 6, "Armature|Death", 21},
};

StateMachine<PosyState> machine(node_list, edge_list);

}  // namespace posy_ai
