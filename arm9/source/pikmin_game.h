#ifndef PIKMIN_GAME_H
#define PIKMIN_GAME_H

#include "multipass_engine.h"
#include "drawable_entity.h"
#include "ai/pikmin.h"
#include "ai/captain.h"
#include "ai/onion.h"
#include "ai/pellet_posy.h"
#include "ui.h"
#include <list>

#include "debug.h"
#include "vram_allocator.h"

struct PikminSave {
  int red_pikmin = 50;
  int yellow_pikmin = 50;
  int blue_pikmin = 50;

  int PikminCount(pikmin_ai::PikminType type);
};

class PikminGame {
 public:
  using SpawnMap = std::map<std::string, std::function<ObjectState*(PikminGame*)>>;

  PikminGame(MultipassEngine& engine);
  ~PikminGame();

  template <typename StateType>
  StateType* SpawnObject();

  template <typename StateType>
  void RemoveObject(StateType* object);

  void Step();
  VramAllocator<Texture>* TextureAllocator();
  VramAllocator<TexturePalette>* TexturePaletteAllocator();
  VramAllocator<Sprite>* SpriteAllocator();

  //useful polling functions
  captain_ai::CaptainState* ActiveCaptain();
  int PikminInField();
  pikmin_ai::PikminState* Pikmin();

  PikminSave* CurrentSaveData();

  //name-based spawning, for level loading and happy debuggers
  template<typename StateType = ObjectState>
  StateType* Spawn(const std::string& name) {
    return reinterpret_cast<StateType*>(spawn_.at(name)(this));
  }

  static std::pair<SpawnMap::const_iterator, SpawnMap::const_iterator> SpawnNames();

 private:
  PikminSave current_save_data_;
  static const SpawnMap spawn_;
  VramAllocator<Texture> texture_allocator_ = VramAllocator<Texture>(VRAM_C, 128 * 1024);
  VramAllocator<TexturePalette> texture_palette_allocator_ = VramAllocator<TexturePalette>(VRAM_G, 16 * 1024, 16);
  VramAllocator<Sprite> sprite_allocator_ = VramAllocator<Sprite>(SPRITE_GFX_SUB, 32 * 1024);
  const u32 kMaxEntities = 256;
  std::list<DrawableEntity*> entities_;

  std::array<pikmin_ai::PikminState, 100> pikmin_;
  std::array<onion_ai::OnionState*, 3> onions_;
  int num_onions_{0};
  std::array<posy_ai::PosyState*, 32> posies_;
  int num_posies_{0};

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
    state->entity->body()->owner = state;
    const bool too_many_objects = state->entity == nullptr;
    if (too_many_objects) {
      delete state;
      return nullptr;
    }
    return state;
  }
};

#endif  // GAME_H
