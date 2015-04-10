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
  // find an available slot for this pikmin
  int slot = 0;
  while (slot < 100 and pikmin_[slot].active) { slot++; }
  if (slot == 100) {
    return nullptr; // fail; can't spawn more pikmin.
  }

  // clear the slot to defaults, then set the ID based on the slot chosen
  pikmin_[slot] = PikminState();
  pikmin_[slot].id = slot;
  pikmin_[slot].active = true;

  // Perform allocation; similar to InitObject, minus the allocation for the
  // state
  pikmin_[slot].entity = allocate_entity();
  pikmin_[slot].game = this;
  const bool too_many_objects = pikmin_[slot].entity == nullptr;
  if (too_many_objects) {    
    return nullptr;
  }
  return &pikmin_[slot];
}

template<>
void Game::RemoveObject<PikminState>(PikminState* object) {
  // similar to cleanup object, again minus the state allocation
  pikmin_[object->id].active = false;
  engine.RemoveEntity(object->entity);
  entities_.remove(object->entity);
  delete object->entity;
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
    pikmin_ai::machine.RunLogic(*i);
    if (i->dead) {
      RemoveObject(i);
    } else {
      i++;
    }
  }
}