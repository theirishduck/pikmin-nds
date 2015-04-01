#include "game.h"

using pikmin_ai::PikminState;
using captain_ai::CaptainState;

Game::Game(MultipassEngine& engine) : engine{engine} {
}

Game::~Game() {
}

VramAllocator* Game::TextureAllocator() {
  return &texture_allocator_;
}

DrawableEntity* Game::allocate_entity() {
  if (entities_.size() >= kMaxEntities) {
    return nullptr;
  }
  entities_.push_back(new DrawableEntity());
  engine.AddEntity(entities_.back());
  return entities_.back();
}

template <>
PikminState* Game::SpawnObject<PikminState>() {
  PikminState* state = InitObject<PikminState>();
  pikmin_.push_back(state);
  return state;
}

template<>
void Game::RemoveObject<PikminState>(PikminState* object) {
  pikmin_.remove(object);
  CleanupObject(object);
}

template <>
CaptainState* Game::SpawnObject<CaptainState>() {
  if (captain_) {
    return captain_;
  }
  captain_ = InitObject<CaptainState>();
  captain_->cursor = allocate_entity();
  return captain_;
}

template<>
void Game::RemoveObject<CaptainState>(CaptainState* object) {
  engine.RemoveEntity(object->cursor);
  entities_.remove(object->cursor);
  delete object->cursor;
  captain_ = nullptr;
  CleanupObject(object);
}

void Game::Step() {
  if (captain_) {
    captain_ai::machine.RunLogic(*captain_);
  }
  
  auto i = pikmin_.begin();
  while (i != pikmin_.end()) {
    auto pikmin_state = *i;
    pikmin_ai::machine.RunLogic(*pikmin_state);
    if (pikmin_state->dead) {
      CleanupObject(pikmin_state);
      pikmin_.erase(i++);
    } else {
      i++;
    }
  }
}