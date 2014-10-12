#ifndef CAPTAIN_H
#define CAPTAIN_H

#include <nds/arm9/videoGL.h>

#include "drawable_entity.h"

class MultipassEngine;

namespace physics {
class Body;
}  // namespace physics

namespace entities {

class Captain : public DrawableEntity {
 public:
  Captain();
  ~Captain();

  void Update() override;
  void Init() override;

 private:
  v16 rotation_{0};
  int running_{true};
  numeric_types::Brads current_angle_ = numeric_types::Brads::Raw(0);

  physics::Body* body_{nullptr};
};

}  // namespace entities

#endif  // CAPTAIN_H
