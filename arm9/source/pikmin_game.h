#ifndef PIKMIN_GAME_H
#define PIKMIN_GAME_H

#include <list>
#include <map>

#include "ai/camera.h"
#include "ai/captain.h"
#include "ai/fire_spout.h"
#include "ai/health.h"
#include "ai/onion.h"
#include "ai/pikmin.h"
#include "ai/pellet_posy.h"
#include "ai/static.h"
#include "ai/treasure.h"
#include "debug/ai_profiler.h"
#include "debug/utilities.h"
#include "debug/dictionary.h"
#include "physics/world.h"
#include "drawable.h"
#include "dsgx_allocator.h"
#include "handle.h"
#include "numeric_types.h"
#include "ui.h"
#include "vector.h"
#include "vram_allocator.h"

class MultipassRenderer;

struct PikminSave {
  int red_pikmin = 100;
  int yellow_pikmin = 100;
  int blue_pikmin = 100;

  int PikminCount(pikmin_ai::PikminType type);
  void AddPikmin(pikmin_ai::PikminType type, int num_pikmin);
};

class PikminGame {
 public:
   enum ObjectType {
     kNone = 0, // Default, useful as an error type. This should never retrieve any object.
     kCaptain,
     kHealth,
     kPikmin,
     kPelletPosy,
     kStatic,
     kTreasure,
     kFireSpout,
     kOnion
   };

  using SpawnMap = std::map<std::string, std::function<PikminGameState*(PikminGame*)>>;

  PikminGame(MultipassRenderer& engine);
  ~PikminGame();

  // Note: While paused, only the UI thread will run. All other logic
  // will cease; the game field should freeze entirely.
  void PauseGame();
  void UnpauseGame();
  bool IsPaused();

  unsigned int CurrentFrame();

  MultipassRenderer& renderer();
  physics::World& world();

  template <typename StateType, unsigned int size>
  Handle SpawnObject(std::array<StateType, size>& object_list, int type);

  template <typename StateType, unsigned int size>
  void RemoveObject(Handle handle, std::array<StateType, size>& object_list);

  void Step();
  VramAllocator<Texture>* TextureAllocator();
  VramAllocator<TexturePalette>* TexturePaletteAllocator();
  VramAllocator<Sprite>* SpriteAllocator();
  DsgxAllocator* ActorAllocator();

  //useful polling functions
  int PikminInField();
  int TotalPikmin();
  std::array<pikmin_ai::PikminState, 100>& PikminList();

  void InitSound(std::string soundbank_filename);

  PikminSave* CurrentSaveData();

  onion_ai::OnionState* Onion(pikmin_ai::PikminType type);

  //name-based spawning, for level loading and happy debuggers
  Handle Spawn(const std::string& name, Vec3 position = Vec3{}, Rotation rotation = Rotation{});

  static std::pair<SpawnMap::const_iterator, SpawnMap::const_iterator> SpawnNames();

  debug::Dictionary& DebugDictionary();
  std::map<std::string, debug::AiProfiler>& DebugAiProfilers();

  // Lists to hold each type of object, and retrieval functions for each
  std::array<captain_ai::CaptainState, 1> captains;
  captain_ai::CaptainState* RetrieveCaptain(Handle handle);

  std::array<fire_spout_ai::FireSpoutState, 16> fire_spouts;
  fire_spout_ai::FireSpoutState* RetrieveFireSpout(Handle handle);

  std::array<onion_ai::OnionState, 3> onions;
  onion_ai::OnionState* RetrieveOnion(Handle handle);

  std::array<pikmin_ai::PikminState, 100> pikmin;
  pikmin_ai::PikminState* RetrievePikmin(Handle handle);

  std::array<posy_ai::PosyState, 32> posies;
  posy_ai::PosyState* RetrievePelletPosy(Handle handle);

  std::array<static_ai::StaticState, 16> statics;
  static_ai::StaticState* RetrieveStatic(Handle handle);

  std::array<treasure_ai::TreasureState, 16> treasures;
  treasure_ai::TreasureState* RetrieveTreasure(Handle handle);

  PikminGameState* Retrieve(Handle handle);

  std::array<health_ai::HealthState, 128> health;
  Handle SpawnHealth();
  health_ai::HealthState* RetrieveHealth(Handle handle);
  void RemoveHealth(Handle handle);

  // The player character is a bit of a special case
  Handle SpawnCaptain();
  void RemoveCaptain(Handle handle);
  Handle ActiveCaptain();

  camera_ai::CameraState& camera();

private:
  physics::World world_;
  int current_generation_ = 0;
  bool paused_ = false;
  PikminSave current_save_data_;
  static const SpawnMap spawn_;
  VramAllocator<Texture> texture_allocator_ = VramAllocator<Texture>(VRAM_C, 128 * 1024);
  VramAllocator<TexturePalette> texture_palette_allocator_ = VramAllocator<TexturePalette>(VRAM_G, 16 * 1024, 16);
  VramAllocator<Sprite> sprite_allocator_ = VramAllocator<Sprite>(SPRITE_GFX_SUB, 32 * 1024);
  DsgxAllocator dsgx_allocator_;
  const u32 kMaxEntities = 256;
  std::list<Drawable*> entities_;
  std::vector<char> soundbank_;

  ui::UIState ui_;
  camera_ai::CameraState camera_;

  Drawable* allocate_entity();
  MultipassRenderer& renderer_;

  void RunAi();

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
  int tPhysicsUpdate;
  // Debug AI Profiler
  std::map<std::string, debug::AiProfiler> ai_profilers_;

  int current_frame_{0};
  int current_step_{0};
};

#endif  // GAME_H
