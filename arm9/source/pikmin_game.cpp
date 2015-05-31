#include "pikmin_game.h"
#include "debug.h"

using pikmin_ai::PikminState;
using captain_ai::CaptainState;
using onion_ai::OnionState;

PikminGame::PikminGame(MultipassEngine& engine) : engine{engine} {
  ui_.game = this;
}

PikminGame::~PikminGame() {
}

VramAllocator* PikminGame::TextureAllocator() {
  return &texture_allocator_;
}

VramAllocator* PikminGame::TexturePaletteAllocator() {
  return &texture_palette_allocator_;
}

VramAllocator* PikminGame::SpriteAllocator() {
  return &sprite_allocator_;
}

DrawableEntity* PikminGame::allocate_entity() {
  if (entities_.size() >= kMaxEntities) {
    return nullptr;
  }
  entities_.push_back(new DrawableEntity());
  engine.AddEntity(entities_.back());
  return entities_.back();
}

template <>
OnionState* PikminGame::SpawnObject<OnionState>() {
  if (num_onions_ < 3) {
    onions_[num_onions_] = InitObject<OnionState>();
    return onions_[num_onions_++];
  }
  return nullptr;
}

template <>
PikminState* PikminGame::SpawnObject<PikminState>() {
  // find an available slot for this pikmin
  int slot = 0;
  while (slot < 100 and pikmin_[slot].active) { slot++; }
  if (slot >= 100) {
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
void PikminGame::RemoveObject<PikminState>(PikminState* object) {
  // similar to cleanup object, again minus the state allocation
  pikmin_[object->id].active = false;
  engine.RemoveEntity(object->entity);
  entities_.remove(object->entity);
  delete object->entity;
}

template <>
CaptainState* PikminGame::SpawnObject<CaptainState>() {
  if (captain_) {
    return captain_;
  }
  captain_ = InitObject<CaptainState>();
  captain_->cursor = allocate_entity();
  captain_->whistle = allocate_entity();
  captain_->squad.captain = captain_;
  return captain_;
}

template<>
void PikminGame::RemoveObject<CaptainState>(CaptainState* object) {
  engine.RemoveEntity(object->cursor);
  entities_.remove(object->cursor);
  delete object->cursor;
  captain_ = nullptr;
  CleanupObject(object);
}

void PikminGame::Step() {
  debug::StartTopic(debug::Topic::kUpdate);
  if (captain_) {
    captain_ai::machine.RunLogic(*captain_);
    squad_ai::machine.RunLogic((*captain_).squad);
  }
  
  auto i = pikmin_.begin();
  while (i != pikmin_.end()) {
    if ((*i).active) {
      pikmin_ai::machine.RunLogic(*i);
    }
    if (i->dead) {
      RemoveObject(i);
    } else {
      i++;
    }
  }

  for (int o = 0; o < num_onions_; o++) {
    onion_ai::machine.RunLogic(*onions_[o]);
  }
  debug::EndTopic(debug::Topic::kUpdate);

  ui::machine.RunLogic(ui_);  
}

CaptainState* PikminGame::ActiveCaptain() {
  return captain_;
}

int PikminGame::PikminInField() {
  int count = 0;
  for (int slot = 0; slot < 100; slot++) {
    if (pikmin_[slot].active) {
      count++;
    }
  }
  return count;
}

PikminState* PikminGame::Pikmin() {
  return &pikmin_[0];
}