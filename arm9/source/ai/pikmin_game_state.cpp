#include "pikmin_game_state.h"
#include "pikmin_game.h"

Vec3 PikminGameState::position() const {
  return body->position;
}

void PikminGameState::set_position(Vec3 position) {
  body->position = position;
}

Vec3 PikminGameState::velocity() const {
  return body->velocity;
}

void PikminGameState::set_velocity(Vec3 velocity) {
  body->velocity = velocity;
}

physics::World& PikminGameState::world() const {
  return game->world();
}

void PikminGameState::Update() {
  entity->set_position(body->position);
}
