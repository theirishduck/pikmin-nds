#ifndef WORLD_H
#define WORLD_H

#include "body.h"
#include "project_settings.h"

namespace physics {

class World {
  public:
    Body* AllocateBody(DrawableEntity* owner);
    void FreeBody(Body* body);
    void Update();
    void DebugCircles();

    void SetHeightmap(const u8* raw_heightmap_data);
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

    numeric_types::fixed HeightFromMap(const Vec3& position);

    physics::Body bodies_[MAX_PHYSICS_BODIES];

    int active_bodies_ = 0;
    int active_[MAX_PHYSICS_BODIES];
    int active_pikmin_ = 0;
    int pikmin_[MAX_PHYSICS_BODIES];

    bool rebuild_index_ = true;
    int heightmap_width = 0;
    int heightmap_height = 0;
    numeric_types::fixed* heightmap_data = nullptr;

    int iteration = 0;
};

}  // namespace physics

#endif
