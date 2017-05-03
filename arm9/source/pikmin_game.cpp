#include "pikmin_game.h"

#include "debug/utilities.h"
#include "dsgx.h"

#include "ai/captain.h"
#include "ai/fire_spout.h"
#include "ai/onion.h"
#include "ai/pellet_posy.h"
#include "ai/pikmin.h"
#include "ai/squad.h"
#include "ai/static.h"
#include "ai/treasure.h"

using pikmin_ai::PikminState;
using pikmin_ai::PikminType;
using captain_ai::CaptainState;
using onion_ai::OnionState;
using posy_ai::PosyState;
using fire_spout_ai::FireSpoutState;
using static_ai::StaticState;
using treasure_ai::TreasureState;

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

int PikminSave::PikminCount(PikminType type) {
  // Note to self: *Probably* shouldn't do it this way
  return ((int*)this)[(int)type - 1];
}

void PikminSave::AddPikmin(PikminType type, int num_pikmin) {
  // Note to self: here too. Seriously. Shame on you.
  ((int*)this)[(int)type - 1] += num_pikmin;
}

int PikminGame::TotalPikmin() {
  int total =
    current_save_data_.PikminCount(PikminType::kRedPikmin) +
    current_save_data_.PikminCount(PikminType::kYellowPikmin) +
    current_save_data_.PikminCount(PikminType::kBluePikmin) +
    PikminInField();
  return total;
}

PikminGame::PikminGame(MultipassEngine& engine) : engine{engine} {
  ui_.game = this;
  ui_.debug_state.game = this;

  // Setup initial debug flags
  engine.debug_flags["Draw Effects Layer"] = false;
  engine.debug_flags["Draw Physics Circles"] = false;
  engine.debug_flags["Skip VBlank"] = false;
  engine.debug_flags["Render First Pass Only"] = false;

  tAI = engine.DebugProfiler().RegisterTopic("Game: AI / Logic");

  ai_profilers_.emplace("Pikmin", debug::AiProfiler());
}

PikminGame::~PikminGame() {
}

MultipassEngine& PikminGame::Engine() {
  return engine;
}

VramAllocator<Texture>* PikminGame::TextureAllocator() {
  return &texture_allocator_;
}

VramAllocator<TexturePalette>* PikminGame::TexturePaletteAllocator() {
  return &texture_palette_allocator_;
}

VramAllocator<Sprite>* PikminGame::SpriteAllocator() {
  return &sprite_allocator_;
}

DsgxAllocator* PikminGame::ActorAllocator() {
  return &dsgx_allocator_;
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
PosyState* PikminGame::SpawnObject<PosyState>() {
  unsigned int slot = 0;
  while (slot < posies_.size() and posies_[slot].active) {
    slot++;
  }
  if (slot >= posies_.size()) {
    return nullptr;
  }

  PosyState& new_posy = posies_[slot];

  // clear the slot to defaults, then set the ID based on the slot chosen
  new_posy = PosyState();
  new_posy.handle.id = slot;
  new_posy.handle.generation = current_generation_;
  new_posy.handle.type = PikminGame::kPelletPosy;

  new_posy.active = true;

  // Perform allocation; similar to InitObject, minus the allocation for the
  // state
  new_posy.entity = allocate_entity();
  new_posy.body = new_posy.entity->body_handle().body;
  new_posy.body->owner = &new_posy;
  new_posy.game = this;
  const bool too_many_objects = new_posy.entity == nullptr;
  if (too_many_objects) {
    return nullptr;
  }

  return &posies_[slot];
}

template <>
StaticState* PikminGame::SpawnObject<StaticState>() {
  unsigned int slot = 0;
  while (slot < statics_.size() and statics_[slot].active) {
    slot++;
  }
  if (slot >= statics_.size()) {
    return nullptr;
  }

  StaticState& new_static = statics_[slot];

  // clear the slot to defaults, then set the ID based on the slot chosen
  new_static = StaticState();
  new_static.handle.id = slot;
  new_static.handle.generation = current_generation_;
  new_static.handle.type = PikminGame::kStatic;

  new_static.active = true;

  // Perform allocation; similar to InitObject, minus the allocation for the
  // state
  new_static.entity = allocate_entity();
  new_static.body = new_static.entity->body_handle().body;
  new_static.body->owner = &new_static;
  new_static.game = this;
  const bool too_many_objects = new_static.entity == nullptr;
  if (too_many_objects) {
    return nullptr;
  }

  return &statics_[slot];
}

template <>
TreasureState* PikminGame::SpawnObject<TreasureState>() {
  unsigned int slot = 0;
  while (slot < treasures_.size() and treasures_[slot].active) {
    slot++;
  }
  if (slot >= posies_.size()) {
    return nullptr;
  }

  TreasureState& new_treasure = treasures_[slot];

  // clear the slot to defaults, then set the ID based on the slot chosen
  new_treasure = TreasureState();
  new_treasure.handle.id = slot;
  new_treasure.handle.generation = current_generation_;
  new_treasure.handle.type = PikminGame::kTreasure;

  new_treasure.active = true;

  // Perform allocation; similar to InitObject, minus the allocation for the
  // state
  new_treasure.entity = allocate_entity();
  new_treasure.body = new_treasure.entity->body_handle().body;
  new_treasure.body->owner = &new_treasure;
  new_treasure.game = this;
  const bool too_many_objects = new_treasure.entity == nullptr;
  if (too_many_objects) {
    return nullptr;
  }

  return &treasures_[slot];
}

template <>
FireSpoutState* PikminGame::SpawnObject<FireSpoutState>() {
  unsigned int slot = 0;
  while (slot < fire_spouts_.size() and fire_spouts_[slot].active) {
    slot++;
  }
  if (slot >= fire_spouts_.size()) {
    return nullptr;
  }

  FireSpoutState& new_fire_spout = fire_spouts_[slot];

  // clear the slot to defaults, then set the ID based on the slot chosen
  new_fire_spout = FireSpoutState();
  new_fire_spout.handle.id = slot;
  new_fire_spout.handle.generation = current_generation_;
  new_fire_spout.handle.type = PikminGame::kFireSpout;

  new_fire_spout.active = true;

  // Perform allocation; similar to InitObject, minus the allocation for the
  // state
  new_fire_spout.entity = allocate_entity();
  new_fire_spout.body = new_fire_spout.entity->body_handle().body;
  new_fire_spout.body->owner = &new_fire_spout;
  new_fire_spout.game = this;
  const bool too_many_objects = new_fire_spout.entity == nullptr;
  if (too_many_objects) {
    return nullptr;
  }

  return &fire_spouts_[slot];
}

template <>
OnionState* PikminGame::SpawnObject<OnionState>() {
  unsigned int slot = 0;
  while (slot < onions_.size() and onions_[slot].active) {
    slot++;
  }
  if (slot >= onions_.size()) {
    return nullptr;
  }

  OnionState& new_onion = onions_[slot];

  // clear the slot to defaults, then set the ID based on the slot chosen
  new_onion = OnionState();
  new_onion.handle.id = slot;
  new_onion.handle.generation = current_generation_;
  new_onion.handle.type = PikminGame::kOnion;

  new_onion.active = true;

  // Perform allocation; similar to InitObject, minus the allocation for the
  // state
  new_onion.entity = allocate_entity();
  new_onion.body = new_onion.entity->body_handle().body;
  new_onion.body->owner = &new_onion;
  new_onion.game = this;
  const bool too_many_objects = new_onion.entity == nullptr;
  if (too_many_objects) {
    return nullptr;
  }

  return &onions_[slot];
}

template <>
PikminState* PikminGame::SpawnObject<PikminState>() {
  // find an available slot for this pikmin
  int slot = 0;
  while (slot < 100 and pikmin_[slot].active) { slot++; }
  if (slot >= 100) {
    return nullptr; // fail; can't spawn more pikmin.
  }

  PikminState& new_pikmin = pikmin_[slot];

  // clear the slot to defaults, then set the ID based on the slot chosen
  new_pikmin = PikminState();
  new_pikmin.handle.id = slot;
  new_pikmin.handle.generation = current_generation_;
  new_pikmin.handle.type = PikminGame::kPikmin;

  new_pikmin.active = true;

  // Perform allocation; similar to InitObject, minus the allocation for the
  // state
  new_pikmin.entity = allocate_entity();
  new_pikmin.body = new_pikmin.entity->body_handle().body;
  new_pikmin.body->owner = &new_pikmin;
  new_pikmin.game = this;
  const bool too_many_objects = new_pikmin.entity == nullptr;
  if (too_many_objects) {
    return nullptr;
  }
  return &pikmin_[slot];
}

template <typename StateType, unsigned int size>
void PikminGame::RemoveObject(Handle handle, std::array<StateType, size>& object_list) {
  if (handle.id < object_list.size()) {
    auto& object_to_delete = object_list[handle.id];
    if (handle.Matches(object_to_delete.handle)) {
      // similar to cleanup object, again minus the state allocation
      object_to_delete.active = false;
      engine.RemoveEntity(object_to_delete.entity);
      entities_.remove(object_to_delete.entity);
      delete object_to_delete.entity;
      current_generation_++;
    } else {
      // Invalid handle! Stale, possibly?
    }
  } else {
    // Invalid ID!
  }
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

void PikminGame::RemoveObject(CaptainState* object) {
  engine.RemoveEntity(object->cursor);
  entities_.remove(object->cursor);
  delete object->cursor;
  captain_ = nullptr;
  CleanupObject(object);
}

void PikminGame::PauseGame() {
  paused_ = true;
  engine.PauseEngine();
}

void PikminGame::UnpauseGame() {
  paused_ = false;
  engine.UnpauseEngine();
}

bool PikminGame::IsPaused() {
  return paused_;
}

void PikminGame::Step() {
  ui::machine.RunLogic(ui_);

  if (paused_) {
    return;
  }

  engine.DebugProfiler().StartTopic(tAI);
  if (captain_) {
    captain_ai::machine.RunLogic(*captain_);
    squad_ai::machine.RunLogic((*captain_).squad);
  }

  ai_profilers_["Pikmin"].ClearTimingData();
  for (auto i = pikmin_.begin(); i != pikmin_.end(); i++) {
    if (i->active) {
      pikmin_ai::machine.RunLogic(*i, &ai_profilers_["Pikmin"]);
      if (i->dead) {
        RemoveObject(i->handle, pikmin_);
      }
    }
  }

  for (unsigned int o = 0; o < onions_.size(); o++) {
    onion_ai::machine.RunLogic(onions_[o]);
  }

  for (unsigned int p = 0; p < posies_.size(); p++) {
    if (posies_[p].active) {
      posy_ai::machine.RunLogic(posies_[p]);
      if (posies_[p].dead) {
        RemoveObject(posies_[p].handle, posies_);
      }
    }
  }

  for (unsigned int f = 0; f < fire_spouts_.size(); f++) {
    if (fire_spouts_[f].active) {
      fire_spout_ai::machine.RunLogic(fire_spouts_[f]);
      if (fire_spouts_[f].dead) {
        RemoveObject(fire_spouts_[f].handle, fire_spouts_);
      }
    }
  }

  for (unsigned int t = 0; t < treasures_.size(); t++) {
    if (treasures_[t].active) {
      treasure_ai::machine.RunLogic(treasures_[t]);
      if (treasures_[t].dead) {
        RemoveObject<TreasureState>(treasures_[t].handle, treasures_);
      }
    }
  }

  engine.DebugProfiler().EndTopic(tAI);

  // Update some debug details about the world
  DebugDictionary().Set("Physics: Bodies Overlapping: ", engine.World().BodiesOverlapping());
  DebugDictionary().Set("Physics: Total Collisions: ", engine.World().TotalCollisions());
}

CaptainState* PikminGame::ActiveCaptain() {
  return captain_;
}

OnionState* PikminGame::Onion(PikminType type) {
  for (unsigned int i = 0; i < onions_.size(); i++) {
    if (onions_[i].pikmin_type == type) {
      return &onions_[i];
    }
  }
  return nullptr;
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

PikminSave* PikminGame::CurrentSaveData() {
  return &current_save_data_;
}

std::array<PikminState, 100>& PikminGame::PikminList() {
  return pikmin_;
}

const std::map<std::string, std::function<ObjectState*(PikminGame*)>> PikminGame::spawn_ = {
  {"Enemy:PelletPosy", [](PikminGame* game) -> ObjectState* {
    return game->SpawnObject<PosyState>();
  }},
  {"Pikmin:Red", [](PikminGame* game) -> ObjectState* {
    auto pikmin = game->SpawnObject<PikminState>();
    pikmin->type = PikminType::kRedPikmin;
    return pikmin;
  }},
  {"Pikmin:Yellow", [](PikminGame* game) -> ObjectState* {
    auto pikmin = game->SpawnObject<PikminState>();
    pikmin->type = PikminType::kYellowPikmin;
    return pikmin;
  }},
  {"Pikmin:Blue", [](PikminGame* game) -> ObjectState* {
    auto pikmin = game->SpawnObject<PikminState>();
    pikmin->type = PikminType::kBluePikmin;
    return pikmin;
  }},
  {"Onion:Red", [](PikminGame* game) -> ObjectState* {
    auto onion = game->SpawnObject<OnionState>();
    onion->pikmin_type = PikminType::kRedPikmin;
    return onion;
  }},
  {"Onion:Yellow", [](PikminGame* game) -> ObjectState* {
    auto onion = game->SpawnObject<OnionState>();
    onion->pikmin_type = PikminType::kYellowPikmin;
    return onion;
  }},
  {"Onion:Blue", [](PikminGame* game) -> ObjectState* {
    auto onion = game->SpawnObject<OnionState>();
    onion->pikmin_type = PikminType::kBluePikmin;
    return onion;
  }},
  {"Hazard:FireSpout", [](PikminGame* game) -> ObjectState* {
    return game->SpawnObject<FireSpoutState>();
  }},
  {"Static", [](PikminGame* game) -> ObjectState* {
    return game->SpawnObject<StaticState>();
  }},
  {"Corpse:Pellet", [](PikminGame* game) -> TreasureState* {
    auto treasure = game->SpawnObject<TreasureState>();
    treasure->entity->set_actor(treasure->game->ActorAllocator()->Retrieve("pellet"));
    treasure->entity->body_handle().body->radius = 2_f;
    return treasure;
  }},
};

std::pair<PikminGame::SpawnMap::const_iterator, PikminGame::SpawnMap::const_iterator> PikminGame::SpawnNames() {
  return std::make_pair(spawn_.begin(), spawn_.end());
}

debug::Dictionary& PikminGame::DebugDictionary() {
  return debug_dictionary_;
}

std::map<std::string, debug::AiProfiler>& PikminGame::DebugAiProfilers() {
  return ai_profilers_;
}
