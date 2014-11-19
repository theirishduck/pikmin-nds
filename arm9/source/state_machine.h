#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include <functional>
#include "drawable_entity.h"

template<typename T>
using GuardFunction = std::function<bool(T const&)>;
template<typename T>
using ActionFunction = std::function<void(T &)>;

struct ObjectState {
  DrawableEntity* entity = nullptr;
  int current_node = 0;
  int frames_alive = 0;
  int frames_at_this_node = 0;
};

enum Trigger {
  kAlways = 0,
  kGuardOnly, // same as always? state logic is run per-frame
  kFirstFrame, //ie, framecount for the state == 0
  kLastFrame, //framecount for the state == duration - 1
};

struct Node {
  const char* name;
  bool can_rest;
  int begin_edge;
  int end_edge;
  const char* animation;
  int duration;
};

template<typename T>
struct Edge {
  Trigger trigger;
  GuardFunction<T> guard;
  ActionFunction<T> action;
  int destination;
};

template <typename T>
class StateMachine {
  public:
    StateMachine(Node* node_list, Edge<T>* edge_list) {
      this->edge_list = edge_list;
      this->node_list = node_list;
    }
    ~StateMachine() {};

    void RunLogic(T& state) {
      auto current_node = node_list[state.current_node];
      for (int i = current_node.begin_edge; i <= current_node.end_edge; i++) {
        auto edge = edge_list[i];
        // Make sure we pass this edge's trigger condition
        if (edge.trigger == kAlways or edge.trigger == kGuardOnly or
            (edge.trigger == kFirstFrame and state.frames_at_this_node == 0) or
            (edge.trigger == kLastFrame and state.frames_at_this_node == current_node.duration - 1)) {
          // If this edge has a guard function, only continue if the guard
          // passes its condition. If not, always continue.
          if (edge.guard == nullptr or edge.guard(state)) {
            // this edge will now be traversed! First, if the edge has an
            // action, run it (this logic typically sets up the next state)
            if (edge.action != nullptr) {
              edge.action(state);
            }

            // update our animation if needed; ie, the new state has animation
            // set, and it's not the animation we're already playing
            if (node_list[edge.destination].animation != nullptr and
                node_list[edge.destination].animation != current_node.animation) {
              state.entity->SetAnimation(node_list[edge.destination].animation);
            }
            // now set our new destination, and reset our counters
            state.current_node = edge.destination;
            state.frames_at_this_node = 0;

            

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
    Edge<T>* edge_list;
    Node* node_list;

};

#endif  // STATE_MACHINE_H_