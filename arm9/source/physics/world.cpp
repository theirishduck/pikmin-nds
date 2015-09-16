#include "world.h"

#include "numeric_types.h"
#include "vector.h"
#include "body.h"
#include "debug.h"

using physics::World;
using physics::Body;
using numeric_types::fixed;
using numeric_types::literals::operator"" _f;

World::World() {
  quadtree = new QuadTree(
    Vec2{0_f, -128_f},
    Vec2{128_f, 0_f}
  );
}

World::~World() {
  delete quadtree;
}

Body* World::AllocateBody(void* owner) {
  // This is a fairly naive implementation.
  // TODO(Nick): See if there's a better way to do this? Alternately, just
  // remember not to spawn 73 things in a single frame.

  // Note: A return value of 0 (Null) indicates failure.
  for (int i = 0; i < MAX_PHYSICS_BODIES; i++) {
    if (bodies_[i].owner == nullptr) {
      bodies_[i].owner = owner;
      bodies_[i].active = 1;
      bodies_[i].height = 1_f;
      bodies_[i].radius = 1_f;
      bodies_[i].collides_with_level = 1;
      bodies_[i].affected_by_gravity = 1;
      bodies_[i].ignores_walls = 0;
      bodies_[i].touching_ground = 0;
      //bodies_[i].radius2 = radius * radius;
      rebuild_index_ = true;

      quadtree->AddObject(&bodies_[i]);
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
  if (&a == &b) {
    return false; // Don't collide with yourself.
  }
  bodies_overlap_debug++;
  //Check to see if the circles overlap on the XZ plane
  Vec2 axz = Vec2{a.position.x, a.position.z};
  Vec2 bxz = Vec2{b.position.x, b.position.z};
  auto distance2 = (axz - bxz).Length2();
  auto sum = a.radius + b.radius;
  auto radius2 = sum * sum;
  if (distance2 < radius2) {
    //Check to see if their Y values are overlapping also
    if (a.position.y + a.height >= b.position.y) {
      if (b.position.y + b.height >= a.position.y) {
        collisions_this_frame++;
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
  if (a.is_movable || b.is_movable) {
    auto a_direction = (a.position - b.position);
    // only resolve on the XZ plane
    a_direction.y = 0_f;
    auto distance = a_direction.Length();

    if (distance == 0_f) {
      //can't correctly resolve this collision, so pick a direction at random
      //to get these objects apart
      //return;
      distance = 1.0_f;
      a_direction.x = 1.0_f;
      a_direction.z = 1.0_f;
    }
    if (a.is_movable and (!(b.is_pikmin) or a.is_pikmin)) {

      // multiply, so that we move exactly the distance required to undo the
      // overlap between these objects
      // a_direction = a_direction.Normalize();
      auto offset = (a.radius + b.radius) - distance;
      if (offset > (b.radius / 8_f)) {
        offset = b.radius / 8_f;
      }

      a_direction *= offset;
      a_direction *= 1_f / distance;

      a.position = a.position + a_direction;
      UpdateBodyInTree(a);
    }
    if (b.is_movable and (!(a.is_pikmin) or b.is_pikmin)) {
      auto b_direction = (b.position - a.position);

      // perform this resolution only on the XZ plane
      b_direction.y = 0_f;

      // multiply, so that we move exactly the distance required to undo the
      // overlap between these objects
      //b_direction = b_direction.Normalize();
      auto offset = (a.radius + b.radius) - distance;
      if (offset > (a.radius / 8_f)) {
        offset = a.radius / 8_f;
      }

      b_direction *= offset;
      b_direction *= 1_f / distance;

      b.position = b.position + b_direction;
      UpdateBodyInTree(b);
    }
  }
}

void World::PrepareBody(Body& body) {
  //set the old position (used later for comparison)
  body.old_position = body.position;
  body.old_radius = body.radius;

  //clear sensor results for this run
  body.result_groups = 0;
  body.num_results = 0;
}

void World::MoveBody(Body& body) {
  body.position += body.velocity;
  body.velocity += body.acceleration;

  // Gravity!
  if (body.affected_by_gravity) {
    body.velocity.y -= GRAVITY_CONSTANT;
  }
}

void World::UpdateBodyInTree(Body& body) {
  if (body.position == body.old_position and
      body.radius == body.old_radius) {
    return;
  }
  body.current_tree->UpdateObject(&body);
}

void World::MoveBodies() {
  for (int i = 0; i < active_bodies_; i++) {
    // First, make sure this is an active body
    PrepareBody(bodies_[active_[i]]);
    MoveBody(bodies_[active_[i]]);
    UpdateBodyInTree(bodies_[active_[i]]);
  }
  for (int i = 0; i < active_pikmin_; i++) {
    // First, make sure this is an active body
    PrepareBody(bodies_[pikmin_[i]]);
    MoveBody(bodies_[pikmin_[i]]);
    UpdateBodyInTree(bodies_[pikmin_[i]]);
  }
}

void World::CollideObjectWithObject(Body& A, Body& B) {
  const bool a_senses_b = B.is_sensor and
      (B.collision_group & A.sensor_groups);
  const bool b_senses_a = A.is_sensor and
      (A.collision_group & B.sensor_groups);

  const bool a_pushes_b = A.collides_with_bodies and not B.is_sensor;
  const bool b_pushes_a = B.collides_with_bodies and not A.is_sensor;

  if (a_senses_b or b_senses_a or a_pushes_b or b_pushes_a) {
    if (BodiesOverlap(A, B)) {
      ResolveCollision(A, B);

      //if A is a sensor that B cares about
      if (A.collision_group & B.sensor_groups) {
        B.result_groups = B.result_groups | A.collision_group;
        if (B.num_results < 8) {
          B.collision_results[B.num_results++] = {&A, A.collision_group};
        }
      }
      //if B is a sensor that A cares about
      if (B.collision_group & A.sensor_groups) {
        A.result_groups = A.result_groups | B.collision_group;
        if (A.num_results < 8) {
          A.collision_results[A.num_results++] = {&B, B.collision_group};
        }
      }
    }
  }
}

void World::CollidePikminWithObject(Body& P, Body& A) {
  if ((A.is_sensor and (A.collision_group & P.sensor_groups)) or
      (not A.is_sensor)) {
    if (BodiesOverlap(A, P)) {
      ResolveCollision(A, P);
      if (A.collision_group & P.sensor_groups) {
        P.result_groups = P.result_groups | A.collision_group;
        if (P.num_results < 8) {
          P.collision_results[P.num_results++] = {&A, A.collision_group};
        }
      }
    }
  }
}

void World::CollidePikminWithPikmin(Body& pikmin1, Body& pikmin2) {
  //if ((int)pikmin1.position.x == (int)pikmin2.position.x and
  //    (int)pikmin1.position.z == (int)pikmin2.position.z) {
  if (BodiesOverlap(pikmin1, pikmin2)) {
    ResolveCollision(pikmin1, pikmin2);
  }
  //}
}

void World::ProcessCollision() {
  for (int a = 0; a < active_bodies_; a++) {
    Body& A = bodies_[a];
    QuadTree* active_quadtree = A.current_tree;
    while (active_quadtree) {
      auto objects = active_quadtree->Objects();
      for (auto b = objects.begin(); b < objects.end(); b++) {
        CollideObjectWithObject(A, **b);
      }
      active_quadtree = active_quadtree->Parent();
    }
  }

  //Repeat this with pikmin, our special case heros
  //Pikmin need to collide against all active bodies, but not with each other

  //Also, pikmin are assumed to collide with both bodies and sensors, and are
  //always considered movable, so we can skip all of those checks.
  /*
  int pikmin_collided = 0;
  for (int p = 0; p < active_pikmin_; p++) {
    Body& P = bodies_[pikmin_[p]];
    QuadTree* active_quadtree = P.current_tree;
    while (active_quadtree) {
      auto objects = active_quadtree->Objects();
      for (auto a = objects.begin(); a < objects.end(); a++) {
        pikmin_collided++;
        CollideObjectWithObject(P, **a);
      }
      active_quadtree = active_quadtree->Parent();
    }
  }
  debug::DisplayValue("Pikmin vs. Objects: ", pikmin_collided);
  */

  // Special case: collide pikmin with each other, but only if they share a
  // heightmap position (later: or an adjacent location?)

  /*
  pikmin_collided = 0;
  for (int p1 = iteration %  8; p1 < active_pikmin_; p1 += 8) {
    Body& pikmin1 = bodies_[pikmin_[p1]];
    QuadTree* active_quadtree = pikmin1.current_tree;
    while (active_quadtree) {
      auto objects = active_quadtree->Objects();
      for (auto pikmin2 = objects.begin(); pikmin2 < objects.end(); pikmin2++) {
        pikmin_collided++;
        CollideObjectWithObject(pikmin1, **pikmin2);
      }
      active_quadtree = active_quadtree->Parent();
    }
  }
  debug::DisplayValue("Pikmin vs. Pikmin: ", pikmin_collided);
  */

}

void World::Update() {
  bodies_overlap_debug = 0;
  collisions_this_frame = 0;
  if (rebuild_index_) {
    RebuildIndex();
  }
  MoveBodies();
  ProcessCollision();
  CollideBodiesWithLevel();

  iteration++;
  debug::DisplayValue("BodiesOverlap Calls: ", bodies_overlap_debug);
  debug::DisplayValue("Total Collisions: ", collisions_this_frame);
}

#include "debug.h"
void World::DebugCircles() {
  for (int i = 0; i < active_bodies_; i++) {
    Body& body = bodies_[active_[i]];

    //pick a color based on the state of this body
    rgb color = RGB5(31,31,31);
    int segments = 16;
    if (body.is_sensor) {
      color = RGB5(31,31,0); //yellow for sensors
    }
    debug::DrawCircle(body.position, body.radius, color, segments);
  }
  for (int i = 0; i < active_pikmin_; i++) {
    Body& body = bodies_[pikmin_[i]];
    //pick a color based on the state of this body
    rgb color = RGB5(31,15,15);
    int segments = 6;
    debug::DrawCircle(body.position, body.radius, color, segments);
  }

  // Also draw the quad tree
  quadtree->DebugDraw();
  debug::DisplayValue("Tree Objects: ", quadtree->ObjectCount());
}

void World::CollideBodiesWithLevel() {
  for (int i = 0; i < active_bodies_; i++) {
    // First, make sure this is an active body
    CollideBodyWithLevel(bodies_[active_[i]]);
  }
  for (int i = 0; i < active_pikmin_; i++) {
    // First, make sure this is an active body
    CollideBodyWithLevel(bodies_[pikmin_[i]]);
  }
}

void World::SetHeightmap(const u8* raw_heightmap_data) {
  heightmap_data = (fixed*)(raw_heightmap_data + 8); // skip over width/height
  int* heightmap_coords = (int*)raw_heightmap_data;
  heightmap_width = heightmap_coords[0];
  heightmap_height = heightmap_coords[1];
}

// Given a world position, figured out the level's height within the loaded
// height map
fixed World::HeightFromMap(const Vec3& position) {
  // Figure out the body's "pixel" within the heightmap; we simply clamp to
  // integers to do this since one pixel is equivalent to one unit in the world
  int hx = (int)position.x;
  int hz = (int)position.z * -1;

  // Clamp the positions to the map edges, so we don't get weirdness
  if (hx < 0) {hx = 0;}
  if (hz < 0) {hz = 0;}
  if (hx >= heightmap_width) {hx = heightmap_width - 1;}
  if (hz >= heightmap_height) {hz = heightmap_height - 1;}

  return heightmap_data[hz * heightmap_width + hx];
}

const fixed kWallThreshold = 2_f; //this seems reasonable

void World::CollideBodyWithLevel(Body& body) {
  if (!body.collides_with_level) {
    return;
  }

  fixed current_level_height = HeightFromMap(body.position);
  if (body.position.y < current_level_height) {
    body.touching_ground = 1; //we touched the ground this frame
    fixed old_level_height = HeightFromMap(body.old_position);
    if (current_level_height - old_level_height > kWallThreshold && !(body.ignores_walls)) {
      // Don't let this object cross the wall! move them back.
      // Note: we ignore Y here to allow gravity to still take effect when
      // running into walls.
      body.position.x = body.old_position.x;
      body.position.z = body.old_position.z;
      // TODO: Reset xz velocity here?
      if (body.position.y < old_level_height) {
        body.position.y = old_level_height;
      }
    } else {
      // Simple case: just adjust their height so they don't sink through the
      // ground, and can walk up slopes and stuff.
      body.position.y = current_level_height;
      // Reset velocity to 0; we hit the ground
      body.velocity.y = 0_f;
    }
  } else {
    body.touching_ground = 0; //we're in the air
  }
}
