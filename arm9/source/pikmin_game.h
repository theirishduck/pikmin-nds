#ifndef PIKMIN_GAME_H
#define PIKMIN_GAME_H

#include "multipass_engine.h"
#include "drawable_entity.h"
//#include "ai/captain.h"
//#include "ai/fire_spout.h"
//#include "ai/onion.h"
//namespace onion_ai {
//struct OnionState;
//}
#include "ai/pikmin.h"
//#include "ai/pellet_posy.h"
//#include "ai/static.h"
//#include "ai/treasure.h"
namespace captain_ai { struct CaptainState; }
namespace fire_spout_ai { struct FireSpoutState; }
namespace onion_ai { struct OnionState; }
namespace posy_ai { struct PosyState;}
namespace treasure_ai { struct TreasureState; }
namespace static_ai { struct StaticState; }
#include "ui.h"
#include <list>

#include "debug/utilities.h"
#include "debug/dictionary.h"
#include "vram_allocator.h"
#include "dsgx_allocator.h"

struct PikminSave {
  int red_pikmin = 100;
  int yellow_pikmin = 100;
  int blue_pikmin = 100;

  int PikminCount(pikmin_ai::PikminType type);
  void AddPikmin(pikmin_ai::PikminType type, int num_pikmin);
};

class PikminGame {
 public:
  using SpawnMap = std::map<std::string, std::function<ObjectState*(PikminGame*)>>;

  PikminGame(MultipassEngine& engine);
  ~PikminGame();

  // Note: While paused, only the UI thread will run. All other logic
  // will cease; the game field should freeze entirely.
  void PauseGame();
  void UnpauseGame();
  bool IsPaused();
  MultipassEngine& Engine();

  template <typename StateType>
  StateType* SpawnObject();

  template <typename StateType>
  void RemoveObject(StateType* object);

  void Step();
  VramAllocator<Texture>* TextureAllocator();
  VramAllocator<TexturePalette>* TexturePaletteAllocator();
  VramAllocator<Sprite>* SpriteAllocator();
  DsgxAllocator* ActorAllocator();

  //useful polling functions
  captain_ai::CaptainState* ActiveCaptain();
  int PikminInField();
  int TotalPikmin();
  pikmin_ai::PikminState* Pikmin();

  PikminSave* CurrentSaveData();

  onion_ai::OnionState* Onion(pikmin_ai::PikminType type);

  //name-based spawning, for level loading and happy debuggers
  template<typename StateType = ObjectState>
  StateType* Spawn(const std::string& name) {
    return reinterpret_cast<StateType*>(spawn_.at(name)(this));
  }

  static std::pair<SpawnMap::const_iterator, SpawnMap::const_iterator> SpawnNames();

  debug::Dictionary& DebugDictionary();
 private:
  bool paused_ = false;
  PikminSave current_save_data_;
  static const SpawnMap spawn_;
  VramAllocator<Texture> texture_allocator_ = VramAllocator<Texture>(VRAM_C, 128 * 1024);
  VramAllocator<TexturePalette> texture_palette_allocator_ = VramAllocator<TexturePalette>(VRAM_G, 16 * 1024, 16);
  VramAllocator<Sprite> sprite_allocator_ = VramAllocator<Sprite>(SPRITE_GFX_SUB, 32 * 1024);
  DsgxAllocator dsgx_allocator_;
  const u32 kMaxEntities = 256;
  std::list<DrawableEntity*> entities_;

  std::array<pikmin_ai::PikminState, 100> pikmin_;
  std::array<onion_ai::OnionState*, 3> onions_;
  int num_onions_{0};
  std::array<posy_ai::PosyState*, 32> posies_;
  int num_posies_{0};
  std::array<fire_spout_ai::FireSpoutState*, 32> fire_spouts_;
  int num_fire_spouts_{0};
  std::array<static_ai::StaticState*, 128> statics_;
  int num_statics_{0};
  std::array<treasure_ai::TreasureState*, 128> treasures_;
  int num_treasures_{0};

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
    state->body = state->entity->body_handle().body;
    const bool too_many_objects = state->entity == nullptr;
    if (too_many_objects) {
      delete state;
      return nullptr;
    }
    state->game = this;
    state->entity->body_handle().body->owner = state;
    return state;
  }

  // Debug Objects
  debug::Dictionary debug_dictionary_;
  // Debug Topic IDs
  int tAI;
};

#endif  // GAME_H
