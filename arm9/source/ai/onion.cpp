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
    onion.feet[i] = onion.entity->engine()->World().AllocateBody(&onion).body;
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
    auto pikmin = onion.game->SpawnObject<pikmin_ai::PikminState>();
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
  // Instead of seeds, spawn pikmin! (todo: not this plz)
  int seeds_to_eject = onion.seeds_count / 2;
  if (seeds_to_eject > 5) {
    seeds_to_eject = 5;
  }
  if (seeds_to_eject == 0 and onion.seeds_count > 0) {
    seeds_to_eject = 1;
  }

  for (int i = 0; i < seeds_to_eject; i++) {
    // Spawn in a pikmin!
    pikmin_ai::PikminState* pikmin = onion.game->SpawnObject<pikmin_ai::PikminState>();
    if (pikmin == nullptr) {
      onion.game->CurrentSaveData()->AddPikmin(onion.pikmin_type, 1);
    } else {
      pikmin->type = onion.pikmin_type;
      pikmin->set_position(onion.position() + Vec3{0_f, 15_f, 0_f});
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
  Edge<OnionState>{kAlways, nullptr, InitAlways, OnionNode::kIdle},
  END_OF_EDGES(OnionState)
};

Edge<OnionState> idle[] {
  {kAlways, SeedsIncreased, UpdateSeedCounter, OnionNode::kBounce},
  {kAlways, nullptr, HandleWithdrawingPikmin, OnionNode::kIdle},  // Loopback
  END_OF_EDGES(OnionState)
};

Edge<OnionState> bounce[] {
  {kAlways, SeedsIncreased, UpdateSeedCounter, OnionNode::kBounce},
  {kLastFrame, nullptr, nullptr, OnionNode::kWindUp},
  END_OF_EDGES(OnionState)
};

Edge<OnionState> wind_up[] {
  {kAlways, SeedsIncreased, UpdateSeedCounter, OnionNode::kBounce},
  {kLastFrame, nullptr, nullptr, OnionNode::kEject},
  END_OF_EDGES(OnionState)
};

Edge<OnionState> wind_down[] {
  {kAlways, SeedsIncreased, UpdateSeedCounter, OnionNode::kBounce},
  {kLastFrame, nullptr, nullptr, OnionNode::kIdle},
  END_OF_EDGES(OnionState)
};

Edge<OnionState> eject_seeds[] {
  {kAlways, SeedsIncreased, UpdateSeedCounter, OnionNode::kBounce},
  {kAlways, NoMoreSeeds, nullptr, OnionNode::kWindDown},
  {kAlways, Every40Frames, EjectSeeds, OnionNode::kEject},  // Loopback
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
