#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H

#include "numeric_types.h"
#include "vector.h"

class DrawableEntity;

namespace physics {

struct Body {
  friend class World;
  //movement information
  Vec3 position;
  Vec3 velocity;
  Vec3 acceleration;

  Vec2 xz_position;

  //all bodies are cylinders, so they have a radius and a height. Their base
  //starts at position.y, so their highest point is at position.y + height.
  numeric_types::Fixed<s32,12> height;
  numeric_types::Fixed<s32,12> radius;
  //numeric_types::Fixed<s32,12> radius2;

  //collision results
  DrawableEntity* owner{nullptr};
  Body* sensor_result{nullptr};

  //collision parameters
  unsigned short is_sensor : 1;  // Does this body inform its colliders?
  unsigned short collides_with_bodies : 1;
  unsigned short collides_with_sensors : 1;
  unsigned short is_movable : 1;  // Can this body be moved during collision?
  unsigned short is_pikmin : 1;  // Pikmin are treated as a special case
  private:
    unsigned short active : 1;
};

}  // namespace physics

#endif  // PHYSICS_BODY_H
