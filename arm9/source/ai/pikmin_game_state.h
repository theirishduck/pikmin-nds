#ifndef AI_PIKMIN_GAME_STATE_H
#define AI_PIKMIN_GAME_STATE_H

#include "handle.h"
#include "state_machine.h"
#include "vector.h"

class PikminGame;
class Drawable;

namespace physics {
  class World;
  class Body;
}

struct PikminGameState : ObjectState {
  Handle handle;
  bool active = false; // Used by the allocator to flag unused slots
  bool dead = false;
  PikminGame* game = nullptr;
  Drawable* entity = nullptr;
  physics::Body* body;

  Vec3 position() const;
  void set_position(Vec3 position);
  Vec3 velocity() const;
  void set_velocity(Vec3 velocity);

  physics::World& world() const;

  void Update();
};

#endif
