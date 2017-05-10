#include "onion.h"

#include "dsgx.h"
#include "pikmin_game.h"
#include "ai/pikmin.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

using pikmin_ai::PikminType;

namespace onion_ai {

void InitAlways(OnionState& onion) {
  onion.entity->set_actor(onion.game->ActorAllocator()->Retrieve("onion"));
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
    onion.feet[i] = onion.game->world().AllocateBody(onion.handle).body;
    onion.feet[i]->radius = 2.1_f;
    onion.feet[i]->height = 1.0_f;
    onion.feet[i]->collision_group = ONION_FEET_GROUP;
  }
  onion.feet[0]->position = onion.position() + Vec3{13.87693_f, 0_f, 0_f};
  onion.feet[1]->position = onion.position() + Vec3{-6.93847_f, 0_f, 12.01778_f};
  onion.feet[2]->position = onion.position() + Vec3{-6.93847_f, 0_f, -12.01778_f};

  // Setup collision for ourself (this is the sensor for the "beam" the captain walks into)
  onion.body->is_sensor = true;
  onion.body->radius = 3.5_f;
  onion.body->height = 3.0_f;
  onion.body->collision_group = ONION_BEAM_GROUP;
}

void HandleWithdrawingPikmin(OnionState& onion) {
  if (onion.withdraw_count > 0) {
    // Spawn in a pikmin of the appropriate type!
    auto pikmin = onion.game->RetrievePikmin(onion.game->Spawn("Pikmin:Red"));
    if (pikmin!= nullptr) {
      pikmin->type = onion.pikmin_type;

      // Set the initial position to one of the sides
      Vec3 onion_sides[] = {
        {4.04651_f, 10.84748_f, -0.06924_f},
        {-1.9633_f, 10.84748_f, 3.539_f},
        {-1.9633_f, 10.84748_f, -3.539_f},
      };

      pikmin->set_position(onion.position() +
          onion_sides[rand() % 3]);

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
      pikmin->entity->body_handle().body->affected_by_gravity = false;
      pikmin->starting_state = pikmin_ai::PikminNode::kSlideDownFromOnion;
      pikmin->entity->RotateToFace(onion.entity);
      auto onion_position = onion.position();
      Vec2 slide_xz = (
          Vec2{onion_position.x, onion_position.z} -
          Vec2{pikmin->position().x, pikmin->position().z}
        ).Normalize();
      slide_xz = slide_xz * -9.2_f;
      pikmin->set_velocity(Vec3{
        slide_xz.x / travel_frames,
        -9.1_f / travel_frames,
        slide_xz.y / travel_frames
      });
    }

    onion.withdraw_count--;
  }
}

bool SeedsIncreased(const OnionState& onion) {
  return onion.seeds_count > onion.old_seeds_count_;
}

void UpdateSeedCounter(OnionState& onion) {
  onion.old_seeds_count_ = onion.seeds_count;
}

bool Every40Frames(const OnionState& onion) {
  return (onion.frames_at_this_node % 40 == 0);
}

void EjectSeeds(OnionState& onion) {
  // Spawn in pikmin seeds, either out in the world, or inside the onion
  int seeds_to_eject = onion.seeds_count / 2;
  if (seeds_to_eject > 5) {
    seeds_to_eject = 5;
  }
  if (seeds_to_eject < onion.seeds_count) {
    seeds_to_eject++;
  }

  for (int i = 0; i < seeds_to_eject; i++) {
    // Spawn in a pikmin, as a seed!
    auto pikmin = onion.game->RetrievePikmin(onion.game->Spawn("Pikmin:Red"));
    if (pikmin == nullptr) {
      onion.game->CurrentSaveData()->AddPikmin(onion.pikmin_type, 1);
    } else {
      pikmin->type = onion.pikmin_type;
      pikmin->set_position(onion.position() + Vec3{0_f, 16_f, 0_f});
      pikmin->starting_state = pikmin_ai::PikminNode::kSeed;
      // pick a random direction for it to float down
      auto direction = Vec2{
        fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12)),
        fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12))
      };
      direction = direction.Normalize();
      pikmin->set_velocity({direction.x * 0.12_f, 0.75_f, direction.y * 0.12_f});
      // Set the Y rotation for the seed appropriately, based on its new direction
      Brads y_rotation = 0_brad;
      if (direction.y <= 0_f) {
        y_rotation = Brads::Raw(acosLerp(direction.x.data_));
      } else {
        y_rotation = Brads::Raw(-acosLerp(direction.x.data_));
      }
      pikmin->entity->set_rotation({0_brad, y_rotation, 0_brad});
    }
  }

  onion.seeds_count -= seeds_to_eject;

  UpdateSeedCounter(onion);
}

bool NoMoreSeeds(const OnionState& onion) {
  return (onion.seeds_count <= 0);
}

namespace OnionNode {
enum OnionNode {
  kInit = 0,
  kIdle,
  kBounce,
  kWindUp,
  kWindDown,
  kEject,
};
}

Edge<OnionState> init[] {
  {Trigger::kAlways, nullptr, InitAlways, OnionNode::kIdle},
  END_OF_EDGES(OnionState)
};

Edge<OnionState> idle[] {
  {Trigger::kAlways, SeedsIncreased, UpdateSeedCounter, OnionNode::kBounce},
  {Trigger::kAlways, nullptr, HandleWithdrawingPikmin, OnionNode::kIdle},  // Loopback
  END_OF_EDGES(OnionState)
};

Edge<OnionState> bounce[] {
  {Trigger::kAlways, SeedsIncreased, UpdateSeedCounter, OnionNode::kBounce},
  {Trigger::kLastFrame, nullptr, nullptr, OnionNode::kWindUp},
  END_OF_EDGES(OnionState)
};

Edge<OnionState> wind_up[] {
  {Trigger::kAlways, SeedsIncreased, UpdateSeedCounter, OnionNode::kBounce},
  {Trigger::kLastFrame, nullptr, nullptr, OnionNode::kEject},
  END_OF_EDGES(OnionState)
};

Edge<OnionState> wind_down[] {
  {Trigger::kAlways, SeedsIncreased, UpdateSeedCounter, OnionNode::kBounce},
  {Trigger::kLastFrame, nullptr, nullptr, OnionNode::kIdle},
  END_OF_EDGES(OnionState)
};

Edge<OnionState> eject_seeds[] {
  {Trigger::kAlways, SeedsIncreased, UpdateSeedCounter, OnionNode::kBounce},
  {Trigger::kAlways, NoMoreSeeds, nullptr, OnionNode::kWindDown},
  {Trigger::kAlways, Every40Frames, EjectSeeds, OnionNode::kEject},  // Loopback
  END_OF_EDGES(OnionState)
};


Node<OnionState> node_list[] {
  {"Init", true, init},
  {"Idle", true, idle, "Armature|Idle", 1},
  {"Bounce", true, bounce, "Armature|Bounce", 9},
  {"WindUp", true, wind_up, "Armature|WindUp", 15},
  {"WindDown", true, wind_down, "Armature|WindDown", 15},
  {"Eject", true, eject_seeds, "Armature|Eject", 20},
};

StateMachine<OnionState> machine(node_list);

}  // namespace onion_ai
