#ifndef GAME_H
#define GAME_H

#include "multipass_engine.h"
#include "drawable_entity.h"
#include "ai/pikmin.h"
#include <list>

#include "debug.h"

class Game {
 public:
  Game(MultipassEngine& engine);
  ~Game();

  template <typename StateType> 
  StateType* SpawnObject();

  template <typename StateType> 
  void RemoveObject(StateType* object);

  void Step();

 private:
  const u32 kMaxEntities = 256;
  std::list<DrawableEntity*> entities_;

  std::list<pikmin_ai::PikminState*> pikmin_;

  DrawableEntity* allocate_entity();
  MultipassEngine& engine;

  template <typename StateType>
  void CleanupObject(StateType* object) {
    engine.RemoveEntity(object->entity);
    debug::nocashNumber(1);
    entities_.remove(object->entity);
    debug::nocashNumber(2);
    delete object->entity;
    debug::nocashNumber(3);
    delete object;
    debug::nocashNumber(4);
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
