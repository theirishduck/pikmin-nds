#include "health.h"

namespace health_ai {

void HealthState::DealDamage(unsigned int amount) {
  int new_health = health - amount;
  health = (new_health < 0 ? 0 : new_health);
}

void HealthState::Heal(unsigned int amount) {
  health = health + amount;
  if (health > max_health) {
    health = max_health;
  }
}

}
