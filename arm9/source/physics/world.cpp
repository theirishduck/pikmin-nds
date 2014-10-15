#include "world.h"

#include "numeric_types.h"
#include "vector.h"
#include "body.h"

using physics::World;
using physics::Body;
using numeric_types::fixed;
using numeric_types::literals::operator"" _f;

Body* World::AllocateBody(DrawableEntity* owner, fixed height, fixed radius) {
  // This is a fairly naive implementation.
  // TODO(Nick): See if there's a better way to do this? Alternately, just
  // remember not to spawn 73 things in a single frame.

  // Note: A return value of 0 (Null) indicates failure.
  for (int i = 0; i < MAX_PHYSICS_BODIES; i++) {
    if (bodies_[i].owner == nullptr) {
      bodies_[i].owner = owner;
      bodies_[i].active = 1;
      bodies_[i].height = height;
      bodies_[i].radius = radius;
      bodies_[i].radius2 = radius * radius;
      rebuild_index_ = true;
      return &bodies_[i];
    }
  }
  return nullptr;
}

void World::FreeBody(Body* body) {
  body->owner = nullptr;
  body->active = 0;
  rebuild_index_ = true;
}

void World::Wake(Body* body) {
  body->active = 1;
  rebuild_index_ = true;
}

void World::Sleep(Body* body) {
  body->active = 0;
  rebuild_index_ = true;
}

void World::RebuildIndex() {
  active_bodies_ = 0;
  active_pikmin_ = 0;
  for (int i = 0; i < MAX_PHYSICS_BODIES; i++) {
    if (bodies_[i].active) {
      if (bodies_[i].is_pikmin) {
        pikmin_[active_pikmin_++] = i;
      } else {
        active_[active_bodies_++] = i;
      }
    }
  }
  rebuild_index_ = false;
}

bool World::BodiesOverlap(Body& a, Body& b) {
  //Check to see if the circles overlap on the XZ plane
  Vec2 axz = Vec2{a.position.x, a.position.z};
  Vec2 bxz = Vec2{b.position.x, b.position.z};
  auto distance2 = (axz - bxz).Length2();
  auto radius2 = (a.radius2 + b.radius2);
  if (distance2 < radius2) {
    //Check to see if their Y values are overlapping also
    if (a.position.y + a.height >= b.position.y) {
      if (b.position.y + b.height >= a.position.y) {
        return true;
      }
    }
  }
  return false;
}

void World::ResolveCollision(Body& a, Body& b) {
  // If either body is a sensor, then no collision response is
  // performed (objects pass right through) so we bail early
  if (a.is_sensor or b.is_sensor) {
    return;
  }
  // One of the bodies must be able to respond to collisions
  if (a.is_movable) {
    auto a_direction = (a.position - b.position);
    // this is intended to be a slow push, so roughly 5% the distance
    // seems appropriate.
    a_direction.y = 0_f;
    a_direction = a_direction * 0.05_f;
    a.position = a.position + a_direction;
  }
  if (b.is_movable) {
    auto b_direction = (b.position - a.position);
    // this is intended to be a slow push, so roughly 5% the distance
    // seems appropriate.
    b_direction = b_direction * 0.05_f;
    b_direction.y = 0_f;
    b.position = b.position + b_direction;
  }
}

void World::MoveBody(Body& body) {
  body.position += body.velocity;
  body.velocity += body.acceleration;

  //clear results from the previous run
  body.sensor_result = 0;
}

void World::MoveBodies() {
  for (int i = 0; i < active_bodies_; i++) {
    // First, make sure this is an active body
    MoveBody(bodies_[active_[i]]);
  }
  for (int i = 0; i < active_pikmin_; i++) {
    // First, make sure this is an active body
    MoveBody(bodies_[pikmin_[i]]);
  }
}

void World::ProcessCollision() {
  int* active_end = active_ + active_bodies_;
  for (int* a = active_; a < active_end; a++) {
    Body& A = bodies_[*a];
    for (int* b = a + 1; b < active_end; b++) {
      if (a != b) {
        Body& B = bodies_[*b];
        if ((A.is_sensor and B.collides_with_sensors) or
            (B.is_sensor and A.collides_with_sensors) or
            (not A.is_sensor and B.collides_with_bodies) or
            (not B.is_sensor and A.collides_with_bodies)) {
          if (BodiesOverlap(A, B)) {
            ResolveCollision(A, B);

            if (A.is_sensor and B.collides_with_sensors) {
              B.sensor_result = &A;
            }
            if (B.is_sensor and A.collides_with_sensors) {
              A.sensor_result = &B;
            }
          }
        }
      }      
    }
  }

  //Repeat this with pikmin, our special case heros
  //Pikmin need to collide against all active bodies, but not with each other
  
  //Also, pikmin are assumed to collide with both bodies and sensors, and are
  //always considered movable, so we can skip all of those checks.
  for (int p = 0; p < active_pikmin_; p++) {
    Body& P = bodies_[pikmin_[p]];
    for (int a = 0; a < active_bodies_; a++) {
      Body& A = bodies_[active_[a]];
      if (BodiesOverlap(A, P)) {
        ResolveCollision(A, P);
        if (A.is_sensor) {
          P.sensor_result = &A;
        }
      }
    }
  }
}

void World::Update() {
  if (rebuild_index_) {
    RebuildIndex();
  }
  MoveBodies();
  ProcessCollision();
}