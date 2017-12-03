#ifndef WORLD_H
#define WORLD_H

#include "body.h"
#include "project_settings.h"

namespace physics {

class World {
  public:
    Body* AllocateBody(Handle owner = Handle{});
    void FreeBody(Body* body);
    Body* RetrieveBody(Handle handle);

    void Update();
    void DebugCircles();

    // Metrics
    int BodiesOverlapping();
    int TotalCollisions();

    void SetHeightmap(const u8* raw_heightmap_data);
    World();
    ~World();

    enum ObjectType {
      kNone = 0,
      kBody
    };
  private:
    bool BodiesOverlap(physics::Body& A, physics::Body& b);
    void ResolveCollision(physics::Body& A, physics::Body& B);
    void ProcessCollision();
    void PrepareBody(Body& body);
    void MoveBody(Body& body);
    void MoveBodies();
    void RebuildIndex();
    void Wake(physics::Body* body);
    void Sleep(physics::Body* body);
    void CollideBodyWithLevel(physics::Body& body);
    void CollideBodiesWithLevel();
    void CollideObjectWithObject(physics::Body& A, physics::Body& B);
    void CollidePikminWithObject(physics::Body& P, physics::Body& A);
    void CollidePikminWithPikmin(physics::Body& pikmin1, physics::Body& pikmin2);
    void UpdateNeighbors();
    void AddNeighborToObject(Body& object, Body& new_neighbor);

    numeric_types::fixed HeightFromMap(const Vec3& position);

    physics::Body bodies_[MAX_PHYSICS_BODIES];

    int active_bodies_ = 0;
    int active_[MAX_PHYSICS_BODIES];
    int active_pikmin_ = 0;
    int pikmin_[MAX_PHYSICS_BODIES];
    int important_bodies_ = 0;
    int important_[MAX_PHYSICS_BODIES];

    bool rebuild_index_ = true;
    int heightmap_width = 0;
    int heightmap_height = 0;
    numeric_types::fixed* heightmap_data = nullptr;

    int iteration = 0;
    int bodies_overlapping_ = 0;
    int total_collisions_ = 0;

    int current_neighbor_ = 0;

    int current_generation_ = 0;

    // Debug Topic IDs
    int tMoveBodies;
    int tCollideBodies;
    int tCollideWorld;
    int tAA;
    int tAP;
    int tPP;
};

}  // namespace physics

#endif
