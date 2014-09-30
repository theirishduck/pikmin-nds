#ifndef YELLOW_PIKMIN_H
#define YELLOW_PIKMIN_H

#include "drawable_entity.h"

class MultipassEngine;

class YellowPikmin : public DrawableEntity {
 public:
  void update(MultipassEngine* engine) override;

 private:
  v16 rotation = 0;
};

#endif  // YELLOW_PIKMIN_H
