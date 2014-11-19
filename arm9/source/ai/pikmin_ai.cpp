#include "pikmin_ai.h"

#include "dsgx.h"
#include "red_pikmin_dsgx.h"
#include "yellow_pikmin_dsgx.h"
#include "blue_pikmin_dsgx.h"

namespace pikmin_ai {

Dsgx red_pikmin_actor((u32*)red_pikmin_dsgx, red_pikmin_dsgx_size);
Dsgx yellow_pikmin_actor((u32*)yellow_pikmin_dsgx, yellow_pikmin_dsgx_size);
Dsgx blue_pikmin_actor((u32*)blue_pikmin_dsgx, blue_pikmin_dsgx_size);

void InitAlways(PikminState& state) {
  switch (state.type) {
    case PikminType::kRedPikmin:
      state.entity->set_actor(&red_pikmin_actor);
      break;
    case PikminType::kYellowPikmin:
      state.entity->set_actor(&yellow_pikmin_actor);
      break;
    case PikminType::kBluePikmin:
      state.entity->set_actor(&blue_pikmin_actor);
      break;
  }
}

void IdleAlways(PikminState& state) {

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
