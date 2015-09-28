#include "pellet_posy.h"

#include "dsgx.h"
#include "pikmin_game.h"
#include "treasure.h"

// Model data
extern const u8 pellet_posy_dsgx[];
extern const u32 pellet_posy_dsgx_size;

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

  // Spawn in the pellet
  auto pellet = posy.game->Spawn<treasure_ai::TreasureState>("Corpse:Pellet");
  pellet->entity->body()->position = posy.entity->body()->position;
}

namespace PosyNode {
enum PosyNode {
  kInit = 0,
  kIdle,
  kHit,
  kDeath,
};
}

Edge<PosyState> init[] {
  // Init
  Edge<PosyState>{kAlways, nullptr, InitAlways, PosyNode::kIdle},
  END_OF_EDGES(PosyState)
};

Edge<PosyState> idle[] {
  // Idle
  {kAlways, ZeroHealth, nullptr, PosyNode::kDeath},
  {kAlways, TookDamage, nullptr, PosyNode::kHit},
  {kAlways, nullptr, nullptr, PosyNode::kIdle},  // Loopback
  END_OF_EDGES(PosyState)
};

Edge<PosyState> hit[] {
  // Hit
  {kAlways, ZeroHealth, nullptr, PosyNode::kDeath},
  {kLastFrame, nullptr, StoreCurrentHealth, PosyNode::kIdle},
  END_OF_EDGES(PosyState)
};

Edge<PosyState> death[] {
  // Death
  {kLastFrame, nullptr, GoodbyeCruelWorld, PosyNode::kDeath},
  END_OF_EDGES(PosyState)
};

Node<PosyState> node_list[] {
  {"Init", true, init},
  {"Idle", true, idle, "Armature|Idle", 30},
  {"Hit", true, hit, "Armature|Hit", 15},
  {"Death", true, death, "Armature|Death", 21},
};

StateMachine<PosyState> machine(node_list);

}  // namespace posy_ai
