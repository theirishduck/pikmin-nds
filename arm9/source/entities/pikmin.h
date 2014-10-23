#ifndef RED_PIKMIN_H
#define RED_PIKMIN_H

#include "multipass_engine.h"

namespace entities {

enum class PikminType {
  kRedPikmin,
  kYellowPikmin,
  kBluePikmin,
};

class Pikmin : public DrawableEntity {
 public:
  Pikmin(PikminType = PikminType::kRedPikmin);
  ~Pikmin();

  void Update() override;
  void Init() override;
  void SetPikminType(PikminType type);

 private:
  bool NeedsNewTarget() const;
  void ChooseNewTarget();
  void Move();

  Dsgx* red_pikmin_actor;
  Dsgx* blue_pikmin_actor;
  Dsgx* yellow_pikmin_actor;

  numeric_types::Brads rotation_;
  bool running_{false};
  s32 updates_until_new_target_{0};
  Vec3 target_;
  Vec3 direction_;

  physics::Body* body_{nullptr};
};

}  // namespace entities 

#endif  // RED_PIKMIN_H
