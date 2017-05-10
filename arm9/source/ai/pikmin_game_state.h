#ifndef AI_PIKMIN_GAME_STATE_H
#define AI_PIKMIN_GAME_STATE_H

#include "state_machine.h"
#include "drawable_entity.h"

class PikminGame;

struct PikminGameState : ObjectState {
  Handle handle;
  bool active = false; // Used by the allocator to flag unused slots
  bool dead = false;
  PikminGame* game = nullptr;
  DrawableEntity* entity = nullptr;
  physics::Body* body;

  Vec3 position() const {
    return body->position;
  };
  void set_position(Vec3 position) {
    body->position = position;
  };
  Vec3 velocity() const {
    return body->velocity;
  };
  void set_velocity(Vec3 velocity) {
    body->velocity = velocity;
  };
};

#endif
