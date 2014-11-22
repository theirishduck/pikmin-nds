#include "pikmin.h"

#include "dsgx.h"
#include "multipass_engine.h"
#include "red_pikmin_dsgx.h"
#include "yellow_pikmin_dsgx.h"
#include "blue_pikmin_dsgx.h"

namespace pikmin_ai {

Dsgx red_pikmin_actor((u32*)red_pikmin_dsgx, red_pikmin_dsgx_size);
Dsgx yellow_pikmin_actor((u32*)yellow_pikmin_dsgx, yellow_pikmin_dsgx_size);
Dsgx blue_pikmin_actor((u32*)blue_pikmin_dsgx, blue_pikmin_dsgx_size);

void InitAlways(PikminState& pikmin) {
  switch (pikmin.type) {
    case PikminType::kRedPikmin:
      pikmin.entity->set_actor(&red_pikmin_actor);
      break;
    case PikminType::kYellowPikmin:
      pikmin.entity->set_actor(&yellow_pikmin_actor);
      break;
    case PikminType::kBluePikmin:
      pikmin.entity->set_actor(&blue_pikmin_actor);
      break;
  }
}

void IdleAlways(PikminState& pikmin) {

}

// This function, assuming unique Pikmin IDs, will return true once every
// 100 frames or so, and will only return true for one pikmin on any given
// frame. This is very handy for making sure that very complex AI tasks aren't
// happening too much in a single frame.
bool AiStaggerDelay(const PikminState& pikmin) {
  return (pikmin.entity->engine()->FrameCounter() % 100) == (pikmin.id % 100);
}

Edge<PikminState> edge_list[] {
  Edge<PikminState>{kAlways, nullptr, InitAlways, 1}, // -> Idle
  {kAlways,nullptr,IdleAlways,1} // -> Idle (loopback)
};

Node node_list[] {
  {"Init", true, 0, 0},
  {"Idle", true, 1, 1, "Armature|Idle", 30}
};

StateMachine<PikminState> machine(node_list, edge_list);

} // namespace pikmin_ai
