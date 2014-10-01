#ifndef CAPTAIN_H
#define CAPTAIN_H

#include <nds/arm9/videoGL.h>

#include "drawable_entity.h"

class MultipassEngine;

class Captain : public DrawableEntity {
 public:
  Captain();
  ~Captain();

  void update(MultipassEngine* engine) override;

 private:
  v16 rotation_{0};
  int running_{true};
  int current_angle_{};
};

#endif  // CAPTAIN_H
