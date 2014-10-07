#ifndef YELLOW_PIKMIN_H
#define YELLOW_PIKMIN_H

#include "drawable_entity.h"

class MultipassEngine;

namespace entities {

class YellowPikmin : public DrawableEntity {
 public:
  void Update(MultipassEngine* engine) override;

 private:
  numeric_types::Brads rotation_;
};

}  // namespace entities

#endif  // YELLOW_PIKMIN_H
