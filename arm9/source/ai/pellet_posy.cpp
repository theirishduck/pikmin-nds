#include "pellet_posy.h"

#include "dsgx.h"
#include "pikmin_game.h"
#include "treasure.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

namespace posy_ai {

void InitAlways(PosyState& posy) {
  posy.entity->set_actor(posy.game->ActorAllocator()->Retrieve("pellet_posy"));

  posy.detection = posy.game->world().AllocateBody(posy.handle);
  posy.detection->position = posy.position();
  posy.detection->radius = 10_f;
  posy.detection->height = 5_f;
  posy.detection->is_sensor = true;
  posy.detection->collision_group = DETECT_GROUP;

  posy.body->collision_group = ATTACK_GROUP;

  auto health_state = posy.game->RetrieveHealth(posy.game->SpawnHealth());
  if (!health_state) {
    posy.dead = true;
    return;
  }
  posy.health_state = health_state;
  posy.old_health = posy.health_state->health;

  posy.body->owner = posy.health_state->handle;
}

bool ZeroHealth(const PosyState& posy) {
  return posy.health_state->health <= 0;
}

bool TookDamage(const PosyState& posy) {
  return posy.health_state->health < posy.old_health;
}

void StoreCurrentHealth(PosyState& posy) {
  posy.old_health = posy.health_state->health;
}

void GoodbyeCruelWorld(PosyState& posy) {
  posy.game->world().FreeBody(posy.detection);
  posy.dead = true;

  // Spawn in the pellet
  auto pellet = posy.game->RetrieveTreasure(posy.game->Spawn("Corpse:Pellet"));
  pellet->set_position(posy.position());
}

void MarkAsDead(PosyState& posy) {
  posy.body->owner = Handle();
  posy.detection->owner = Handle();
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
  {Trigger::kAlways, nullptr, InitAlways, PosyNode::kIdle},
  END_OF_EDGES(PosyState)
};

Edge<PosyState> idle[] {
  // Idle
  {Trigger::kAlways, ZeroHealth, MarkAsDead, PosyNode::kDeath},
  {Trigger::kAlways, TookDamage, nullptr, PosyNode::kHit},
  {Trigger::kAlways, nullptr, nullptr, PosyNode::kIdle},  // Loopback
  END_OF_EDGES(PosyState)
};

Edge<PosyState> hit[] {
  // Hit
  {Trigger::kAlways, ZeroHealth, MarkAsDead, PosyNode::kDeath},
  {Trigger::kLastFrame, nullptr, StoreCurrentHealth, PosyNode::kIdle},
  END_OF_EDGES(PosyState)
};

Edge<PosyState> death[] {
  // Death
  {Trigger::kLastFrame, nullptr, GoodbyeCruelWorld, PosyNode::kDeath},
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
