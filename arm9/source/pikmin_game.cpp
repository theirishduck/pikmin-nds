#include "pikmin_game.h"

#include "debug/draw.h"
#include "debug/flags.h"
#include "dsgx.h"

using pikmin_ai::PikminState;
using pikmin_ai::PikminType;
using captain_ai::CaptainState;
using onion_ai::OnionState;
using posy_ai::PosyState;
using fire_spout_ai::FireSpoutState;
using health_ai::HealthState;
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

PikminGame::PikminGame(MultipassRenderer& renderer) : renderer_{renderer} {
  camera_.game = this;
  ui_.game = this;
  ui_.debug_state.game = this;

  // Setup initial debug flags
  debug::RegisterFlag("Draw Effects Layer");
  debug::RegisterFlag("Draw Physics Circles");
  debug::RegisterFlag("Draw Renderer Circles");
  debug::RegisterFlag("Skip VBlank");
  debug::RegisterFlag("Render First Pass Only");

  debug::RegisterWorld(&world_);
  debug::RegisterRenderer(&renderer_);

  tAI = renderer_.DebugProfiler().RegisterTopic("Game: AI / Logic");
  tPhysicsUpdate = renderer_.DebugProfiler().RegisterTopic("Game: Physics");

  ai_profilers_.emplace("Pikmin", debug::AiProfiler());
}

PikminGame::~PikminGame() {
}

MultipassRenderer& PikminGame::renderer() {
  return renderer_;
}

physics::World& PikminGame::world() {
  return world_;
}

camera_ai::CameraState& PikminGame::camera() {
  return camera_;
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

Drawable* PikminGame::allocate_entity() {
  if (entities_.size() >= kMaxEntities) {
    return nullptr;
  }
  entities_.push_back(new Drawable());
  renderer_.AddEntity(entities_.back());
  return entities_.back();
}

unsigned int PikminGame::CurrentFrame() {
  return current_frame_;
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

  new_object.entity = allocate_entity();
  new_object.body = world_.AllocateBody(new_object.handle);
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
      renderer_.RemoveEntity(object_to_delete.entity);
      entities_.remove(object_to_delete.entity);
      delete object_to_delete.entity;
      world_.FreeBody(object_to_delete.body);
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
    renderer_.RemoveEntity(captain->cursor);
    entities_.remove(captain->cursor);
    delete captain->cursor;

    renderer_.RemoveEntity(captain->whistle);
    entities_.remove(captain->whistle);
    delete captain->whistle;

    RemoveObject(handle, captains);
  }
}

Handle PikminGame::SpawnHealth() {
  for (unsigned int i = 0; i < health.size(); i++) {
    if (!(health[i].active)) {
      health[i] = HealthState();
      health[i].handle.id = i;
      health[i].handle.generation = current_generation_;
      health[i].handle.type = PikminGame::kHealth;
      health[i].active = true;
      return health[i].handle;
    }
  }
  return Handle();
}

HealthState* PikminGame::RetrieveHealth(Handle handle) {
  if (handle.id < health.size()) {
    auto object = &health[handle.id];
    if (object->active and object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

void PikminGame::RemoveHealth(Handle handle) {
  HealthState* object = RetrieveHealth(handle);
  if (object) {
    object->active = false;
    object->handle = Handle();
  }
}

CaptainState* PikminGame::RetrieveCaptain(Handle handle) {
  if (handle.id < captains.size()) {
    auto object = &captains[handle.id];
    if (object->active and object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

FireSpoutState* PikminGame::RetrieveFireSpout(Handle handle) {
  if (handle.id < fire_spouts.size()) {
    auto object = &fire_spouts[handle.id];
    if (object->active and object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

OnionState* PikminGame::RetrieveOnion(Handle handle) {
  if (handle.id < onions.size()) {
    auto object = &onions[handle.id];
    if (object->active and object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

PikminState* PikminGame::RetrievePikmin(Handle handle) {
  if (handle.id < pikmin.size()) {
    auto object = &pikmin[handle.id];
    if (object->active and object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

PosyState* PikminGame::RetrievePelletPosy(Handle handle) {
  if (handle.id < posies.size()) {
    auto object = &posies[handle.id];
    if (object->active and object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

StaticState* PikminGame::RetrieveStatic(Handle handle) {
  if (handle.id < statics.size()) {
    auto object = &statics[handle.id];
    if (object->active and object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

TreasureState* PikminGame::RetrieveTreasure(Handle handle) {
  if (handle.id < treasures.size()) {
    auto object = &treasures[handle.id];
    if (object->active and object->handle.Matches(handle)) {
      return object;
    }
  }
  return nullptr;
}

void PikminGame::PauseGame() {
  paused_ = true;
  renderer_.PauseEngine();
}

void PikminGame::UnpauseGame() {
  paused_ = false;
  renderer_.UnpauseEngine();
}

bool PikminGame::IsPaused() {
  return paused_;
}

void PikminGame::RunAi() {
  renderer_.DebugProfiler().StartTopic(tAI);
  for (auto i = captains.begin(); i != captains.end(); i++) {
    if (i->active) {
      captain_ai::machine.RunLogic(*i);
      squad_ai::machine.RunLogic((*i).squad);
      i->Update();
      i->whistle->set_position(i->whistle_body->position);
      i->cursor->set_position(i->cursor_body->position);
      if (i->dead) {
        RemoveCaptain(i->handle);
      }
    }
  }

  ai_profilers_["Pikmin"].ClearTimingData();
  for (unsigned int i = 0; i < pikmin.size(); i++) {
    if (pikmin[i].active) {
      //pikmin_ai::machine.RunLogic(pikmin[i], &ai_profilers_["Pikmin"]);
      pikmin_ai::machine.RunLogic(pikmin[i]);
      pikmin[i].Update();
      if (pikmin[i].dead) {
        RemoveObject(pikmin[i].handle, pikmin);
      }
    }
  }

  for (unsigned int o = 0; o < onions.size(); o++) {
    onion_ai::machine.RunLogic(onions[o]);
    onions[o].Update();
  }

  for (unsigned int p = 0; p < posies.size(); p++) {
    if (posies[p].active) {
      posy_ai::machine.RunLogic(posies[p]);
      posies[p].Update();
      if (posies[p].dead) {
        RemoveObject(posies[p].handle, posies);
      }
    }
  }

  for (unsigned int f = 0; f < fire_spouts.size(); f++) {
    if (fire_spouts[f].active) {
      fire_spout_ai::machine.RunLogic(fire_spouts[f]);
      fire_spouts[f].Update();
      if (fire_spouts[f].dead) {
        RemoveObject(fire_spouts[f].handle, fire_spouts);
      }
    }
  }

  for (unsigned int t = 0; t < treasures.size(); t++) {
    if (treasures[t].active) {
      treasure_ai::machine.RunLogic(treasures[t]);
      treasures[t].Update();
      if (treasures[t].dead) {
        RemoveObject<TreasureState>(treasures[t].handle, treasures);
      }
    }
  }

  camera_ai::machine.RunLogic(camera_);

  renderer_.DebugProfiler().EndTopic(tAI);
}

void PikminGame::Step() {
  current_step_++;
  if (current_step_ % 2 == 0) {
    // On even frames, run AI
    ui::machine.RunLogic(ui_);

    if (IsPaused()) {
      return;
    }

    RunAi();
    current_frame_++;
  } else {
    // On odd frames, run the World, and update the engine bits
    renderer_.Update();

    if (!IsPaused()) {
      renderer_.DebugProfiler().StartTopic(tPhysicsUpdate);
      world_.Update();
      renderer_.DebugProfiler().EndTopic(tPhysicsUpdate);

      // Update some debug details about the world
      DebugDictionary().Set("Physics: Bodies Overlapping: ", world().BodiesOverlapping());
      DebugDictionary().Set("Physics: Total Collisions: ", world().TotalCollisions());
    }
  }
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

const std::map<std::string, std::function<PikminGameState*(PikminGame*)>> PikminGame::spawn_ = {
  {"Enemy:PelletPosy", [](PikminGame* game) -> PikminGameState* {
    return game->RetrievePelletPosy(game->SpawnObject(game->posies, PikminGame::kPelletPosy));
  }},
  {"Pikmin:Red", [](PikminGame* game) -> PikminGameState* {
    auto pikmin = game->RetrievePikmin(game->SpawnObject(game->pikmin, PikminGame::kPikmin));
    if (pikmin) {
      pikmin->type = PikminType::kRedPikmin;
    }
    return pikmin;
  }},
  {"Pikmin:Yellow", [](PikminGame* game) -> PikminGameState* {
    auto pikmin = game->RetrievePikmin(game->SpawnObject(game->pikmin, PikminGame::kPikmin));
    if (pikmin) {
      pikmin->type = PikminType::kYellowPikmin;
    }
    return pikmin;
  }},
  {"Pikmin:Blue", [](PikminGame* game) -> PikminGameState* {
    auto pikmin = game->RetrievePikmin(game->SpawnObject(game->pikmin, PikminGame::kPikmin));
    if (pikmin) {
      pikmin->type = PikminType::kBluePikmin;
    }
    return pikmin;
  }},
  {"Onion:Red", [](PikminGame* game) -> PikminGameState* {
    auto onion = game->RetrieveOnion(game->SpawnObject(game->onions, PikminGame::kOnion));
    if (onion) {
      onion->pikmin_type = PikminType::kRedPikmin;
    }
    return onion;
  }},
  {"Onion:Yellow", [](PikminGame* game) -> PikminGameState* {
    auto onion = game->RetrieveOnion(game->SpawnObject(game->onions, PikminGame::kOnion));
    if (onion) {
      onion->pikmin_type = PikminType::kYellowPikmin;
    }
    return onion;
  }},
  {"Onion:Blue", [](PikminGame* game) -> PikminGameState* {
    auto onion = game->RetrieveOnion(game->SpawnObject(game->onions, PikminGame::kOnion));
    if (onion) {
      onion->pikmin_type = PikminType::kBluePikmin;
    }
    return onion;
  }},
  {"Hazard:FireSpout", [](PikminGame* game) -> PikminGameState* {
    return game->RetrieveFireSpout(game->SpawnObject(game->fire_spouts, PikminGame::kFireSpout));
  }},
  {"Static", [](PikminGame* game) -> PikminGameState* {
    return game->RetrieveStatic(game->SpawnObject(game->statics, PikminGame::kStatic));
  }},
  {"Corpse:Pellet:Red", [](PikminGame* game) -> PikminGameState* {
    auto treasure = game->RetrieveTreasure(game->SpawnObject(game->treasures, PikminGame::kTreasure));
    if (treasure) {
      treasure->pikmin_affinity = PikminType::kRedPikmin;
      treasure->entity->set_actor(treasure->game->ActorAllocator()->Retrieve("pellet"));
      treasure->body->radius = 2_f;

      treasure->weight = 1;
      treasure->carry_slots = 2;
      treasure->pikmin_seeds = 2;
    }
    return treasure;
  }},
};

std::pair<PikminGame::SpawnMap::const_iterator, PikminGame::SpawnMap::const_iterator> PikminGame::SpawnNames() {
  return std::make_pair(spawn_.begin(), spawn_.end());
}

Handle PikminGame::Spawn(const std::string& name, Vec3 position, Rotation rotation) {
  PikminGameState* object = spawn_.at(name)(this);
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
