#ifndef YELLOW_PIKMIN_H
#define YELLOW_PIKMIN_H

#include "drawable_entity.h"

class MultipassEngine;

class YellowPikmin : public DrawableEntity {
 public:
  void Update(MultipassEngine* engine) override;

 private:
  numeric_types::Brads rotation_;
};

#endif  // YELLOW_PIKMIN_H
