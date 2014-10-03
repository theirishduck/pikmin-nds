#ifndef RED_PIKMIN_H
#define RED_PIKMIN_H

#include "multipass_engine.h"

class RedPikmin : public DrawableEntity {
 public:
  RedPikmin();
  ~RedPikmin();

  void update(MultipassEngine* engine) override;

 private:
  bool NeedsNewTarget() const;
  void ChooseNewTarget();
  void Move();

  numeric_types::Brads rotation_;
  bool running_{false};
  s32 updates_until_new_target_{0};
  Vec3 target_;
  Vec3 direction_;
};

#endif  // RED_PIKMIN_H
