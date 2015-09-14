#include "onion.h"

#include "dsgx.h"
#include "pikmin_game.h"

// Model data
#include "redonion_dsgx.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

namespace onion_ai {

Dsgx redonion_actor((u32*)redonion_dsgx, redonion_dsgx_size);

void InitAlways(OnionState& onion) {
  onion.entity->set_actor(&redonion_actor);
  redonion_actor.ApplyTextures(onion.game->TextureAllocator(), onion.game->TexturePaletteAllocator());

  // Setup collision for feet
  for (int i = 0; i < 3; i++) {
    onion.feet[i] = onion.entity->engine()->World().AllocateBody(&onion);
    onion.feet[i]->radius = 1.0_f;
    onion.feet[i]->height = 1.0_f;
    onion.feet[i]->collision_group = ONION_FEET_GROUP;
  }
  onion.feet[0]->position = onion.entity->body()->position + Vec3{13.87693_f, 0_f, 0_f};
  onion.feet[1]->position = onion.entity->body()->position + Vec3{-6.93847_f, 0_f, 12.01778_f};
  onion.feet[2]->position = onion.entity->body()->position + Vec3{-6.93847_f, 0_f, -12.01778_f};

  // Setup collision for ourself (this is the sensor for the "beam" the captain walks into)
  onion.entity->body()->is_sensor = true;
  onion.entity->body()->radius = 3.5_f;
  onion.entity->body()->height = 3.0_f;
  onion.entity->body()->collision_group = ONION_BEAM_GROUP;
}

Edge<OnionState> edge_list[] {
  // Init
  Edge<OnionState>{kAlways, nullptr, InitAlways, 1},

  // Idle
  {kAlways, nullptr, nullptr, 1},  // Loopback
};

Node node_list[] {
  {"Init", true, 0, 0},
  {"Idle", true, 1, 1},
};

StateMachine<OnionState> machine(node_list, edge_list);

}  // namespace onion_ai
