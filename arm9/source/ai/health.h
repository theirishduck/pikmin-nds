#ifndef AI_HEALTH_H
#define AI_HEALTH_H

#include "handle.h"

namespace health_ai {

struct HealthState {
  Handle handle;
  bool active{false};
  unsigned int max_health{100};
  unsigned int health{100};

  void DealDamage(unsigned int amount);
  void Heal(unsigned int amount);
};

}  // namespace squad_ai

#endif
