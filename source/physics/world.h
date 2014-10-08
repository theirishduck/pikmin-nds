#ifndef WORLD_H
#define WORLD_H

namespace physics {

class Body;

class World {
  public:

  private:
    bool BodiesOverlap(physics::Body& A, physics::Body& b);
    void ResolveCollision(physics::Body& A, physics::Body& B);
    void ProcessCollision();
    void MoveBodies();
};

}  // namespace physics

#endif
