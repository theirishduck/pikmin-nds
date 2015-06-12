#include "pellet_posy.h"

#include "dsgx.h"
#include "pikmin_game.h"

// Model data
#include "pellet_posy_dsgx.h"

namespace posy_ai {

Dsgx posy_actor((u32*)pellet_posy_dsgx, pellet_posy_dsgx_size);

void InitAlways(PosyState& posy) {
  posy.entity->set_actor(&posy_actor);
  posy_actor.ApplyTextures(posy.game->TextureAllocator(), posy.game->TexturePaletteAllocator());
}

Edge<PosyState> edge_list[] {
  // Init
  Edge<PosyState>{kAlways, nullptr, InitAlways, 1},

  // Idle
  {kAlways, nullptr, nullptr, 1},  // Loopback
};

Node node_list[] {
  {"Init", true, 0, 0},
  {"Idle", true, 1, 1, "Armature|Idle", 60},
};

StateMachine<PosyState> machine(node_list, edge_list);

}  // namespace posy_ai
