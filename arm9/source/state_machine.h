#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include <functional>
#include "drawable_entity.h"

template<typename T>
using GuardFunction = std::function<bool(T const&)>;
template<typename T>
using ActionFunction = std::function<void(T &)>;

class PikminGame;

struct ObjectState {
  PikminGame* game = nullptr;
  DrawableEntity* entity = nullptr;
  int current_node = 0;
  int frames_alive = 0;
  int frames_at_this_node = 0;
  bool dead = false;

  Vec3 position() const {
    return entity->body_handle().body->position;
  };
  void set_position(Vec3 position) {
    entity->body_handle().body->position = position;
  };
  Vec3 velocity() const {
    return entity->body_handle().body->velocity;
  };
  void set_velocity(Vec3 velocity) {
    entity->body_handle().body->velocity = velocity;
  };
};

enum Trigger {
  kAlways = 0,
  kGuardOnly, // same as always? state logic is run per-frame
  kFirstFrame, //ie, framecount for the state == 0
  kLastFrame, //framecount for the state == duration - 1
  kEndOfList,
};

template<typename T>
struct Edge {
  Trigger trigger;
  GuardFunction<T> guard;
  ActionFunction<T> action;
  int destination;
};

#define END_OF_EDGES(Type) Edge<Type>{kEndOfList, nullptr, nullptr, 0},

template<typename T>
struct Node {
  const char* name;
  bool can_rest;
  Edge<T>* edge_list;
  const char* animation;
  int duration;
};

template <typename T>
class StateMachine {
  public:
    StateMachine(Node<T>* node_list) {
      this->node_list = node_list;
    }
    ~StateMachine() {};

    const char* NodeName(int node) {
      return node_list[node].name;
    }

    void RunLogic(T& state) {
      auto current_node = node_list[state.current_node];
      for (auto i = current_node.edge_list; i->trigger != kEndOfList; i++) {
        auto edge = *i;
        // Make sure we pass this edge's trigger condition
        if (edge.trigger == kAlways or edge.trigger == kGuardOnly or
            (edge.trigger == kFirstFrame and state.frames_at_this_node == 0) or
            (edge.trigger == kLastFrame and state.frames_at_this_node >= current_node.duration - 1)) {
          // If this edge has a guard function, only continue if the guard
          // passes its condition. If not, always continue.
          if (edge.guard == nullptr or edge.guard(state)) {
            // Only reset our frames_at_this_node if the transition takes
            // us to a *different* node; this prevents loopback transitions
            // from resetting the counter to 0 every frame.
            if (state.current_node != edge.destination) {
              state.frames_at_this_node = -1; // this is incremented to 0 down there
            }
            // now set our new destination
            state.current_node = edge.destination;

            // Run the action for this state, if any. This runs last, so it has
            // the ability to override any of the above logic if needed.
            if (edge.action != nullptr) {
              edge.action(state);
            }

            // update our animation if needed; ie, the new state has animation
            // set, and it's not the animation we're already playing
            if (node_list[state.current_node].animation != nullptr and
                node_list[state.current_node].animation != current_node.animation) {
              state.entity->SetAnimation(node_list[state.current_node].animation);
            }

            //finally, break out so we stop processing edges
            break;
          }
        }
      }

      //increment counters, to track actions and lifetimes
      state.frames_alive++;
      state.frames_at_this_node++;
    }

  private:
    Node<T>* node_list;

};

#endif  // STATE_MACHINE_H_
