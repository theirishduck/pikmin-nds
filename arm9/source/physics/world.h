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

  private:
    bool BodiesOverlap(physics::Body& A, physics::Body& b);
    void ResolveCollision(physics::Body& A, physics::Body& B);
    void ProcessCollision();
    void MoveBody(Body& body);
    void MoveBodies();
    void RebuildIndex();
    void Wake(physics::Body* body);
    void Sleep(physics::Body* body);

    physics::Body bodies_[MAX_PHYSICS_BODIES];

    int active_bodies_ = 0;
    int active_[MAX_PHYSICS_BODIES];
    int active_pikmin_ = 0;
    int pikmin_[MAX_PHYSICS_BODIES];

    bool rebuild_index_ = true;
};

}  // namespace physics

#endif
