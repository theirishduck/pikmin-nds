#include "onion.h"

#include "dsgx.h"
#include "pikmin_game.h"
#include "ai/pikmin.h"

// Model data
#include "onion_dsgx.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

using pikmin_ai::PikminType;

namespace onion_ai {

Dsgx onion_actor((u32*)onion_dsgx, onion_dsgx_size);

void InitAlways(OnionState& onion) {
  onion.entity->set_actor(&onion_actor);
  onion_actor.ApplyTextures(onion.game->TextureAllocator(), onion.game->TexturePaletteAllocator());
  if (onion.pikmin_type == PikminType::kRedPikmin) {
    onion.entity->set_mesh("RedOnion");
  }
  if (onion.pikmin_type == PikminType::kYellowPikmin) {
    onion.entity->set_mesh("YellowOnion");
  }
  if (onion.pikmin_type == PikminType::kBluePikmin) {
    onion.entity->set_mesh("BlueOnion");
  }

  // Setup collision for feet
  for (int i = 0; i < 3; i++) {
    onion.feet[i] = onion.entity->engine()->World().AllocateBody(&onion);
    onion.feet[i]->radius = 2.1_f;
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

      if (onion.pikmin_type == PikminType::kRedPikmin) {
        onion.game->CurrentSaveData()->red_pikmin--;
      }
      if (onion.pikmin_type == PikminType::kYellowPikmin) {
        onion.game->CurrentSaveData()->yellow_pikmin--;
      }
      if (onion.pikmin_type == PikminType::kBluePikmin) {
        onion.game->CurrentSaveData()->blue_pikmin--;
      }

      // Setup some animation data on the pikmin, so it can slide away from
      // the onion toward one of the feet
      fixed travel_frames = 30_f;
      pikmin->entity->body()->affected_by_gravity = false;
      pikmin->starting_state = pikmin_ai::PikminNode::kSlideDownFromOnion;
      pikmin->entity->RotateToFace(onion.entity);
      auto pikmin_body = pikmin->entity->body();
      auto onion_position = onion.entity->body()->position;
      Vec2 slide_xz = (
          Vec2{onion_position.x, onion_position.z} -
          Vec2{pikmin_body->position.x, pikmin_body->position.z}
        ).Normalize();
      slide_xz = slide_xz * -9.2_f;
      pikmin->entity->body()->velocity = Vec3{
        slide_xz.x / travel_frames,
        -9.1_f / travel_frames,
        slide_xz.y / travel_frames
      };
    }

    onion.withdraw_count--;
  }
}

Edge<OnionState> init[] {
  Edge<OnionState>{kAlways, nullptr, InitAlways, 1},
  END_OF_EDGES(OnionState)
};

Edge<OnionState> idle[] {
  {kAlways, nullptr, HandleWithdrawingPikmin, 1},  // Loopback
  END_OF_EDGES(OnionState)
};

Node<OnionState> node_list[] {
  {"Init", true, init},
  {"Idle", true, idle},
};

StateMachine<OnionState> machine(node_list);

}  // namespace onion_ai
