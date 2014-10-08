#ifndef WORLD_H
#define WORLD_H

#include "body.h"
#include "project_settings.h"

namespace physics {

class World {
  public:
    Body* AllocateBody(DrawableEntity* owner);
    void FreeBody(Body* body);

    physics::Body bodies[MAX_PHYSICS_BODIES];

  private:
    bool BodiesOverlap(physics::Body& A, physics::Body& b);
    void ResolveCollision(physics::Body& A, physics::Body& B);
    void ProcessCollision();
    void MoveBodies();
};

}  // namespace physics

#endif
