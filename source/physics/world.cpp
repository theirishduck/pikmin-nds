#include "world.h"

#include "numeric_types.h"
#include "vector.h"
#include "body.h"

using physics::World;
using numeric_types::Fixed;
using numeric_types::literals::operator"" _f;

bool World::BodiesOverlap(Body& a, Body& b) {
  //Check to see if the circles overlap on the XZ plane
  Vec2 axz = Vec2{a.position.x, a.position.z};
  Vec2 bxz = Vec2{b.position.x, b.position.z};
  auto distance2 = (axz - bxz).Length2();
  auto radius2 = (a.radius * a.radius + b.radius * b.radius);
  if (distance2 < radius2) {
    //Check to see if their Y values are overlapping also
    if (a.position.y + a.height >= a.position.y) {
      if (b.position.y + b.height >= b.position.y) {
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
        a.position = a.position + a_direction;
      }
      if (b.is_movable) {
        auto b_direction = (b.position - a.position);
        //this is intended to be a slow push, so roughly 10% the distance
        //seems appropriate.
        b_direction = b_direction * 0.1_f;
        b.position = b.position + b_direction;
      }
    }
  }
}

void World::MoveBodies() {
  
}

void World::ProcessCollision() {

}

