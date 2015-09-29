#include "body.h"

using physics::Body;

physics::CollisionResult Body::FirstCollisionWith(u32 collision_mask) {
  for (int i = 0; i < 8; i++) {
    if (collision_results[i].collision_group & collision_mask) {
      return collision_results[i];
    }
  }
  return CollisionResult();
}
