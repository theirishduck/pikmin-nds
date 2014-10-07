#ifndef CAPTAIN_H
#define CAPTAIN_H

#include <nds/arm9/videoGL.h>

#include "drawable_entity.h"

class MultipassEngine;

namespace entities {

class Captain : public DrawableEntity {
 public:
  Captain();
  ~Captain();

  void Update(MultipassEngine* engine) override;

 private:
  v16 rotation_{0};
  int running_{true};
  numeric_types::Brads current_angle_ = numeric_types::Brads::Raw(0);
};

}  // namespace entities

#endif  // CAPTAIN_H
