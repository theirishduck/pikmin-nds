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

bool physics::BodyHandle::is_valid() {
  if (!body or !body->active or body->generation != this->generation) {
    return false;
  }
  return true;
}
