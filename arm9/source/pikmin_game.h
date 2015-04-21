#ifndef PIKMIN_GAME_H
#define PIKMIN_GAME_H

#include "multipass_engine.h"
#include "drawable_entity.h"
#include "ai/pikmin.h"
#include "ai/captain.h"
#include "ai/onion.h"
#include "ui.h"
#include <list>

#include "debug.h"
#include "vram_allocator.h"

class PikminGame {
 public:
  PikminGame(MultipassEngine& engine);
  ~PikminGame();

  template <typename StateType> 
  StateType* SpawnObject();

  template <typename StateType> 
  void RemoveObject(StateType* object);

  void Step();
  VramAllocator* TextureAllocator();
  VramAllocator* SpriteAllocator();

  //useful polling functions
  captain_ai::CaptainState* ActiveCaptain();

 private:
  VramAllocator texture_allocator_ = VramAllocator(VRAM_C, 128 * 1024);
  VramAllocator sprite_allocator_ = VramAllocator(SPRITE_GFX_SUB, 32 * 1024);
  const u32 kMaxEntities = 256;
  std::list<DrawableEntity*> entities_;

  std::array<pikmin_ai::PikminState, 100> pikmin_;
  std::array<onion_ai::OnionState*, 3> onions_;
  int num_onions_{0};
  captain_ai::CaptainState* captain_;
  ui::UIState ui_;

  DrawableEntity* allocate_entity();
  MultipassEngine& engine;

  template <typename StateType>
  void CleanupObject(StateType* object) {
    engine.RemoveEntity(object->entity);
    entities_.remove(object->entity);
    delete object->entity;
    delete object;
  }

  template <typename StateType>
  StateType* InitObject() {
    StateType* state = new StateType();
    state->entity = allocate_entity();
    state->game = this;
    const bool too_many_objects = state->entity == nullptr;
    if (too_many_objects) {
      delete state;
      return nullptr;
    }
    return state;
  }
};

#endif  // GAME_H
