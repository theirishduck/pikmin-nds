#include "onion.h"

#include "dsgx.h"
#include "pikmin_game.h"

// Model data
#include "redonion_dsgx.h"

namespace onion_ai {

Dsgx redonion_actor((u32*)redonion_dsgx, redonion_dsgx_size);

void InitAlways(OnionState& onion) {
  onion.entity->set_actor(&redonion_actor);
  redonion_actor.ApplyTextures(onion.game->TextureAllocator(), onion.game->TexturePaletteAllocator());
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
