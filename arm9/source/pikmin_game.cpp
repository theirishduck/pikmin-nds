#include "pikmin_game.h"
#include "debug.h"
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
}

PikminGame::~PikminGame() {
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
  if (num_posies_ < 32) {
    posies_[num_posies_] = InitObject<PosyState>();
    return posies_[num_posies_++];
  }
  return nullptr;
}

template<>
void PikminGame::RemoveObject<PosyState>(PosyState* object) {
  object->active = false;
  engine.RemoveEntity(object->entity);
  entities_.remove(object->entity);
  delete object->entity;
}

template <>
StaticState* PikminGame::SpawnObject<StaticState>() {
  if (num_statics_ < 128) {
    statics_[num_statics_] = InitObject<StaticState>();
    return statics_[num_statics_++];
  }
  return nullptr;
}

template<>
void PikminGame::RemoveObject<StaticState>(StaticState* object) {
  engine.RemoveEntity(object->entity);
  entities_.remove(object->entity);
  delete object->entity;
}

template <>
TreasureState* PikminGame::SpawnObject<TreasureState>() {
  if (num_treasures_ < 128) {
    treasures_[num_treasures_] = InitObject<TreasureState>();
    return treasures_[num_treasures_++];
  }
  return nullptr;
}

template<>
void PikminGame::RemoveObject<TreasureState>(TreasureState* object) {
  engine.RemoveEntity(object->entity);
  entities_.remove(object->entity);
  delete object->entity;
  object->active = false;
}

template <>
FireSpoutState* PikminGame::SpawnObject<FireSpoutState>() {
  if (num_fire_spouts_ < 32) {
    fire_spouts_[num_fire_spouts_] = InitObject<FireSpoutState>();
    return fire_spouts_[num_fire_spouts_++];
  }
  return nullptr;
}

template<>
void PikminGame::RemoveObject<FireSpoutState>(FireSpoutState* object) {
  object->active = false;
  engine.RemoveEntity(object->entity);
  entities_.remove(object->entity);
  delete object->entity;
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
  pikmin_[slot].body = pikmin_[slot].entity->body_handle().body;
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
  nocashMessage("Remove Pikmin Called");
  pikmin_[object->id].active = false;
  nocashMessage("Set Pikmin Inactive Succeeded");
  engine.RemoveEntity(object->entity);
  nocashMessage("Removed Pikmin from Engine!");
  entities_.remove(object->entity);
  nocashMessage("Removed Pikmin from Entities list!");
  delete object->entity;
  nocashMessage("Deleted Entity successfully!");
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

  debug::StartTopic(debug::Topic::kAI);
  if (captain_) {
    captain_ai::machine.RunLogic(*captain_);
    squad_ai::machine.RunLogic((*captain_).squad);
  }

  for (auto i = pikmin_.begin(); i != pikmin_.end(); i++) {
    if ((*i).active) {
      pikmin_ai::machine.RunLogic(*i);
      debug::DisplayValue("NodeFrames", i->frames_at_this_node);
      debug::DisplayValue("Node", pikmin_ai::machine.NodeName(i->current_node));
      if (i->dead) {
        RemoveObject(i);
      }
    }
  }

  for (int o = 0; o < num_onions_; o++) {
    onion_ai::machine.RunLogic(*onions_[o]);
  }

  for (int p = 0; p < num_posies_; p++) {
    if (posies_[p]->active) {
      posy_ai::machine.RunLogic(*posies_[p]);
      if (posies_[p]->dead) {
        RemoveObject<PosyState>(posies_[p]);
      }
    }
  }

  for (int f = 0; f < num_fire_spouts_; f++) {
    if (fire_spouts_[f]->active) {
      fire_spout_ai::machine.RunLogic(*fire_spouts_[f]);
      if (fire_spouts_[f]->dead) {
        RemoveObject<FireSpoutState>(fire_spouts_[f]);
      }
    }
  }

  for (int t = 0; t < num_treasures_; t++) {
    if (treasures_[t]->active) {
      treasure_ai::machine.RunLogic(*treasures_[t]);
      if (treasures_[t]->dead) {
        RemoveObject<TreasureState>(treasures_[t]);
      }
    }
  }

  debug::EndTopic(debug::Topic::kAI);
}

CaptainState* PikminGame::ActiveCaptain() {
  return captain_;
}

OnionState* PikminGame::Onion(PikminType type) {
  for (int i = 0; i < num_onions_; i++) {
    if (onions_[i]->pikmin_type == type) {
      return onions_[i];
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

PikminState* PikminGame::Pikmin() {
  return &pikmin_[0];
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
