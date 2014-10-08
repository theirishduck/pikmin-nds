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
    if (bodies[i].owner == nullptr) {
      bodies[i].owner = owner;
      bodies[i].active = 1;
      bodies[i].height = height;
      bodies[i].radius = radius;
      return &bodies[i];
    }
  }
  return nullptr;
}

void World::FreeBody(Body* body) {
  body->owner = nullptr;
  body->active = 0;
}

bool World::BodiesOverlap(Body& a, Body& b) {
  //Check to see if the circles overlap on the XZ plane
  Vec2 axz = Vec2{a.position.x, a.position.z};
  Vec2 bxz = Vec2{b.position.x, b.position.z};
  auto distance2 = (axz - bxz).Length2();
  auto radius2 = (a.radius * a.radius + b.radius * b.radius);
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
  //If either body is a sensor, bail
  if (not (a.is_sensor or b.is_sensor)) {
    // One of the bodies must be able to respond to collisions
    if (a.is_movable or b.is_movable) {
      if (a.is_movable) {
        auto a_direction = (a.position - b.position);
        //this is intended to be a slow push, so roughly 10% the distance
        //seems appropriate.
        a_direction = a_direction * 0.1_f;
        a_direction.y = 0_f;
        a.position = a.position + a_direction;
      }
      if (b.is_movable) {
        auto b_direction = (b.position - a.position);
        //this is intended to be a slow push, so roughly 10% the distance
        //seems appropriate.
        b_direction = b_direction * 0.1_f;
        b_direction.y = 0_f;
        b.position = b.position + b_direction;
      }
    }
  }
}

void World::MoveBodies() {
  for (int i = 0; i < MAX_PHYSICS_BODIES; i++) {
    // First, make sure this is an active body
    if (bodies[i].owner and bodies[i].active) {
      bodies[i].position += bodies[i].velocity;
      bodies[i].velocity += bodies[i].acceleration;

      //clear results from the previous run
      bodies[i].sensor_result = 0;
    }
  }
}

void World::ProcessCollision() {
  for (int a = 0; a < MAX_PHYSICS_BODIES; a++) {
    for (int b = a + 1; b < MAX_PHYSICS_BODIES; b++) {
      if (a != b and bodies[a].active and bodies[b].active) {
        if ((bodies[a].is_sensor and bodies[b].collides_with_sensors) or
            (bodies[b].is_sensor and bodies[a].collides_with_sensors) or
            (not bodies[a].is_sensor and bodies[b].collides_with_bodies) or
            (not bodies[b].is_sensor and bodies[a].collides_with_bodies)) {
          if (BodiesOverlap(bodies[a], bodies[b])) {
            ResolveCollision(bodies[a], bodies[b]);

            if (bodies[a].is_sensor and bodies[b].collides_with_sensors) {
              bodies[b].sensor_result = &bodies[a];
            }
            if (bodies[b].is_sensor and bodies[a].collides_with_sensors) {
              bodies[a].sensor_result = &bodies[b];
            }
          }
        }
      }      
    }
  }
}

void World::Update() {
  MoveBodies();
  ProcessCollision();
}