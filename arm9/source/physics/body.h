#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H

#include <array>

#include "handle.h"
#include "numeric_types.h"
#include "project_settings.h"
#include "vector.h"

namespace physics {

struct Body;

struct BodyHandle {
  Body* body{nullptr};
  int generation{0};

  bool IsValid() const;
};

struct CollisionResult {
  Body* body;
  u32 collision_group;
};

struct Neighbor {
  BodyHandle handle;
  numeric_types::Fixed<s32,12> distance = numeric_types::Fixed<s32,12>::FromInt(0);
};

struct Body {
  Handle handle;

  friend class World;
  //movement information
  Vec3 position = Vec3{
    numeric_types::Fixed<s32,12>::FromInt(0),
    numeric_types::Fixed<s32,12>::FromInt(0),
    numeric_types::Fixed<s32,12>::FromInt(0)
  };
  Vec3 velocity = Vec3{
    numeric_types::Fixed<s32,12>::FromInt(0),
    numeric_types::Fixed<s32,12>::FromInt(0),
    numeric_types::Fixed<s32,12>::FromInt(0)
  };;
  Vec3 acceleration = Vec3{
    numeric_types::Fixed<s32,12>::FromInt(0),
    numeric_types::Fixed<s32,12>::FromInt(0),
    numeric_types::Fixed<s32,12>::FromInt(0)
  };;

  Vec2 xz_position = Vec2{
    numeric_types::Fixed<s32,12>::FromInt(0),
    numeric_types::Fixed<s32,12>::FromInt(0)
  };;

  //all bodies are cylinders, so they have a radius and a height. Their base
  //starts at position.y, so their highest point is at position.y + height.
  numeric_types::Fixed<s32,12> height = numeric_types::Fixed<s32,12>::FromInt(1);
  numeric_types::Fixed<s32,12> radius = numeric_types::Fixed<s32,12>::FromInt(1);
  //numeric_types::Fixed<s32,12> radius2;

  // Stores a handle to the owner of this body, helpful when reacting to
  // collisions.
  Handle owner;

  //list of which collision groups we BELONG TO
  u32 collision_group{0};

  //list of all groups which we WANT TO KNOW ABOUT
  u32 sensor_groups{0};

  //contains all the groups we collided with *this frame*
  u32 result_groups{0};

  u32 num_results{0};
  CollisionResult collision_results[8];

  unsigned short touching_ground : 1;

  //collision parameters
  unsigned short is_sensor : 1;  // Does this body inform its colliders?
  unsigned short collides_with_bodies : 1;
  unsigned short collides_with_level : 1;
  unsigned short ignores_walls : 1;
  unsigned short is_movable : 1;  // Can this body be moved during collision?
  unsigned short is_pikmin : 1;  // Pikmin are treated as a special case
  unsigned short affected_by_gravity : 1;
  unsigned short is_very_important : 1;

  CollisionResult FirstCollisionWith(u32 collision_mask);
  unsigned short active : 1;
  unsigned short generation;
  Vec3 old_position;
  numeric_types::Fixed<s32,12> old_radius;

  std::array<Neighbor, MAX_PHYSICS_NEIGHBORS> neighbors;

  BodyHandle GetHandle();
};

}  // namespace physics

#endif  // PHYSICS_BODY_H
