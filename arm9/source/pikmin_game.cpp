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

template <typename StateType, unsigned int size>
Handle PikminGame::SpawnObject(std::array<StateType, size>& object_list, int type) {
  unsigned int slot = 0;
  while (slot < object_list.size() and object_list[slot].active) {
    slot++;
  }
  if (slot >= object_list.size()) {
    return Handle();
  }

  StateType& new_object = object_list[slot];

  // clear the slot to defaults, then set the ID based on the slot chosen
  new_object = StateType();
  new_object.handle.id = slot;
  new_object.handle.generation = current_generation_;
  new_object.handle.type = type;

  new_object.active = true;

  // Perform allocation; similar to InitObject, minus the allocation for the
  // state
  new_object.entity = allocate_entity();
  new_object.body = new_object.entity->body_handle().body;
  new_object.body->owner = new_object.handle;
  new_object.game = this;
  const bool too_many_objects = new_object.entity == nullptr;
  if (too_many_objects) {
    return Handle();
  }

  return new_object.handle;
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

Handle PikminGame::SpawnCaptain() {
  CaptainState* captain = RetrieveCaptain(SpawnObject(captains, PikminGame::kCaptain));
  if (captain) {
    captain->cursor = allocate_entity();
    captain->whistle = allocate_entity();
    captain->squad.captain = captain;
  } else {
    // How did we fail here?
  }
  return captain->handle;
}

void PikminGame::RemoveCaptain(Handle handle) {
  CaptainState* captain = RetrieveCaptain(handle);
  if (captain) {
    engine.RemoveEntity(captain->cursor);
    entities_.remove(captain->cursor);
    delete captain->cursor;

    engine.RemoveEntity(captain->whistle);
    entities_.remove(captain->whistle);
    delete captain->whistle;

    RemoveObject(handle, captains);
  }
}

CaptainState* PikminGame::RetrieveCaptain(Handle handle) {
  if (handle.id < captains.size()) {
    auto object = &captains[handle.id];
    if (object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

FireSpoutState* PikminGame::RetrieveFireSpout(Handle handle) {
  if (handle.id < fire_spouts.size()) {
    auto object = &fire_spouts[handle.id];
    if (object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

OnionState* PikminGame::RetrieveOnion(Handle handle) {
  if (handle.id < onions.size()) {
    auto object = &onions[handle.id];
    if (object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

PikminState* PikminGame::RetrievePikmin(Handle handle) {
  if (handle.id < pikmin.size()) {
    auto object = &pikmin[handle.id];
    if (object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

PosyState* PikminGame::RetrievePelletPosy(Handle handle) {
  if (handle.id < posies.size()) {
    auto object = &posies[handle.id];
    if (object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

StaticState* PikminGame::RetrieveStatic(Handle handle) {
  if (handle.id < statics.size()) {
    auto object = &statics[handle.id];
    if (object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

TreasureState* PikminGame::RetrieveTreasure(Handle handle) {
  if (handle.id < treasures.size()) {
    auto object = &treasures[handle.id];
    if (object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
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
  for (auto i = captains.begin(); i != captains.end(); i++) {
    if (i->active) {
      captain_ai::machine.RunLogic(*i);
      squad_ai::machine.RunLogic((*i).squad);
      if (i->dead) {
        RemoveCaptain(i->handle);
      }
    }
  }

  ai_profilers_["Pikmin"].ClearTimingData();
  for (auto i = pikmin.begin(); i != pikmin.end(); i++) {
    if (i->active) {
      pikmin_ai::machine.RunLogic(*i, &ai_profilers_["Pikmin"]);
      if (i->dead) {
        RemoveObject(i->handle, pikmin);
      }
    }
  }

  for (unsigned int o = 0; o < onions.size(); o++) {
    onion_ai::machine.RunLogic(onions[o]);
  }

  for (unsigned int p = 0; p < posies.size(); p++) {
    if (posies[p].active) {
      posy_ai::machine.RunLogic(posies[p]);
      if (posies[p].dead) {
        RemoveObject(posies[p].handle, posies);
      }
    }
  }

  for (unsigned int f = 0; f < fire_spouts.size(); f++) {
    if (fire_spouts[f].active) {
      fire_spout_ai::machine.RunLogic(fire_spouts[f]);
      if (fire_spouts[f].dead) {
        RemoveObject(fire_spouts[f].handle, fire_spouts);
      }
    }
  }

  for (unsigned int t = 0; t < treasures.size(); t++) {
    if (treasures[t].active) {
      treasure_ai::machine.RunLogic(treasures[t]);
      if (treasures[t].dead) {
        RemoveObject<TreasureState>(treasures[t].handle, treasures);
      }
    }
  }

  engine.DebugProfiler().EndTopic(tAI);

  // Update some debug details about the world
  DebugDictionary().Set("Physics: Bodies Overlapping: ", engine.World().BodiesOverlapping());
  DebugDictionary().Set("Physics: Total Collisions: ", engine.World().TotalCollisions());
}

Handle PikminGame::ActiveCaptain() {
  return captains[0].handle;
}

OnionState* PikminGame::Onion(PikminType type) {
  for (unsigned int i = 0; i < onions.size(); i++) {
    if (onions[i].pikmin_type == type) {
      return &onions[i];
    }
  }
  return nullptr;
}

int PikminGame::PikminInField() {
  int count = 0;
  for (int slot = 0; slot < 100; slot++) {
    if (pikmin[slot].active) {
      count++;
    }
  }
  return count;
}

PikminSave* PikminGame::CurrentSaveData() {
  return &current_save_data_;
}

std::array<PikminState, 100>& PikminGame::PikminList() {
  return pikmin;
}

const std::map<std::string, std::function<ObjectState*(PikminGame*)>> PikminGame::spawn_ = {
  {"Enemy:PelletPosy", [](PikminGame* game) -> ObjectState* {
    return game->RetrievePelletPosy(game->SpawnObject(game->posies, PikminGame::kPelletPosy));
  }},
  {"Pikmin:Red", [](PikminGame* game) -> ObjectState* {
    auto pikmin = game->RetrievePikmin(game->SpawnObject(game->pikmin, PikminGame::kPikmin));
    pikmin->type = PikminType::kRedPikmin;
    return pikmin;
  }},
  {"Pikmin:Yellow", [](PikminGame* game) -> ObjectState* {
    auto pikmin = game->RetrievePikmin(game->SpawnObject(game->pikmin, PikminGame::kPikmin));
    pikmin->type = PikminType::kYellowPikmin;
    return pikmin;
  }},
  {"Pikmin:Blue", [](PikminGame* game) -> ObjectState* {
    auto pikmin = game->RetrievePikmin(game->SpawnObject(game->pikmin, PikminGame::kPikmin));
    pikmin->type = PikminType::kBluePikmin;
    return pikmin;
  }},
  {"Onion:Red", [](PikminGame* game) -> ObjectState* {
    auto onion = game->RetrieveOnion(game->SpawnObject(game->onions, PikminGame::kOnion));
    onion->pikmin_type = PikminType::kRedPikmin;
    return onion;
  }},
  {"Onion:Yellow", [](PikminGame* game) -> ObjectState* {
    auto onion = game->RetrieveOnion(game->SpawnObject(game->onions, PikminGame::kOnion));
    onion->pikmin_type = PikminType::kYellowPikmin;
    return onion;
  }},
  {"Onion:Blue", [](PikminGame* game) -> ObjectState* {
    auto onion = game->RetrieveOnion(game->SpawnObject(game->onions, PikminGame::kOnion));
    onion->pikmin_type = PikminType::kBluePikmin;
    return onion;
  }},
  {"Hazard:FireSpout", [](PikminGame* game) -> ObjectState* {
    return game->RetrieveFireSpout(game->SpawnObject(game->fire_spouts, PikminGame::kFireSpout));
  }},
  {"Static", [](PikminGame* game) -> ObjectState* {
    return game->RetrieveStatic(game->SpawnObject(game->statics, PikminGame::kStatic));
  }},
  {"Corpse:Pellet", [](PikminGame* game) -> ObjectState* {
    auto treasure = game->RetrieveTreasure(game->SpawnObject(game->treasures, PikminGame::kTreasure));
    treasure->entity->set_actor(treasure->game->ActorAllocator()->Retrieve("pellet"));
    treasure->entity->body_handle().body->radius = 2_f;
    return treasure;
  }},
};

std::pair<PikminGame::SpawnMap::const_iterator, PikminGame::SpawnMap::const_iterator> PikminGame::SpawnNames() {
  return std::make_pair(spawn_.begin(), spawn_.end());
}

Handle PikminGame::Spawn(const std::string& name, Vec3 position, Rotation rotation) {
  ObjectState* object = spawn_.at(name)(this);
  object->set_position(position);
  object->entity->set_rotation(rotation);
  return object->handle;
}

debug::Dictionary& PikminGame::DebugDictionary() {
  return debug_dictionary_;
}

std::map<std::string, debug::AiProfiler>& PikminGame::DebugAiProfilers() {
  return ai_profilers_;
}
