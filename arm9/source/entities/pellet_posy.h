#ifndef PELLET_POSY_H
#define PELLET_POSY_H

#include "drawable_entity.h"

class MultipassEngine;

namespace physics {
class Body;
}  // namespace physics

namespace entities {

class PelletPosy : public DrawableEntity {
  public:
    PelletPosy();
    ~PelletPosy();
  private:
    physics::Body* body_{nullptr};
};

}  // namespace entities

#endif // PELLET_POSY_H