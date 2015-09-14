#include "onion.h"

#include "dsgx.h"
#include "pikmin_game.h"
#include "ai/pikmin.h"

// Model data
#include "redonion_dsgx.h"
#include "yellowonion_dsgx.h"
#include "blueonion_dsgx.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

using pikmin_ai::PikminType;

namespace onion_ai {

Dsgx redonion_actor((u32*)redonion_dsgx, redonion_dsgx_size);
Dsgx yellowonion_actor((u32*)yellowonion_dsgx, yellowonion_dsgx_size);
Dsgx blueonion_actor((u32*)blueonion_dsgx, blueonion_dsgx_size);

void InitAlways(OnionState& onion) {
  if (onion.pikmin_type == PikminType::kRedPikmin) {
    onion.entity->set_actor(&redonion_actor);
    redonion_actor.ApplyTextures(onion.game->TextureAllocator(), onion.game->TexturePaletteAllocator());
  }
  if (onion.pikmin_type == PikminType::kYellowPikmin) {
    onion.entity->set_actor(&yellowonion_actor);
    yellowonion_actor.ApplyTextures(onion.game->TextureAllocator(), onion.game->TexturePaletteAllocator());
  }
  if (onion.pikmin_type == PikminType::kBluePikmin) {
    onion.entity->set_actor(&blueonion_actor);
    blueonion_actor.ApplyTextures(onion.game->TextureAllocator(), onion.game->TexturePaletteAllocator());
  }

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

void HandleWithdrawingPikmin(OnionState& onion) {
  if (onion.withdraw_count > 0) {
    // Spawn in a pikmin of the appropriate type!
    auto pikmin = onion.game->SpawnObject<pikmin_ai::PikminState>();
    if (pikmin!= nullptr) {
      pikmin->type = onion.pikmin_type;

      // Set the initial position to one of the sides
      Vec3 onion_sides[] = {
        {4.04651_f, 10.84748_f, -0.06924_f},
        {-1.9633_f, 10.84748_f, 3.539_f},
        {-1.9633_f, 10.84748_f, -3.539_f},
      };

      pikmin->entity->body()->position = onion.entity->body()->position +
          onion_sides[rand() % 3];

      // For now, go ahead and add this pikmin to the captain's squad
      onion.game->ActiveCaptain()->squad.AddPikmin(pikmin);

      if (onion.pikmin_type == PikminType::kRedPikmin) {
        onion.game->CurrentSaveData()->red_pikmin--;
      }
      if (onion.pikmin_type == PikminType::kYellowPikmin) {
        onion.game->CurrentSaveData()->yellow_pikmin--;
      }
      if (onion.pikmin_type == PikminType::kBluePikmin) {
        onion.game->CurrentSaveData()->blue_pikmin--;
      }
    }

    onion.withdraw_count--;
  }
}

Edge<OnionState> edge_list[] {
  // Init
  Edge<OnionState>{kAlways, nullptr, InitAlways, 1},

  // Idle
  {kAlways, nullptr, HandleWithdrawingPikmin, 1},  // Loopback
};

Node node_list[] {
  {"Init", true, 0, 0},
  {"Idle", true, 1, 1},
};

StateMachine<OnionState> machine(node_list, edge_list);

}  // namespace onion_ai
