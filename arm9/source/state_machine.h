#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include <functional>

#include "debug/ai_profiler.h"
#include "drawable.h"

template<typename T>
using GuardFunction = bool (*)(T const&);
template<typename T>
using ActionFunction = void (*)(T &);

struct ObjectState {
  int current_node = 0;
  int frames_alive = 0;
  int frames_at_this_node = 0;
  Drawable* entity = nullptr;
};

enum class Trigger {
  kAlways = 0,
  kGuardOnly, // same as always? state logic is run per-frame
  kFirstFrame, //ie, framecount for the state == 0
  kLastFrame, //framecount for the state == duration - 1
  kEndOfList,
};

#define kNoGuard nullptr
#define kNoAction nullptr

template<typename T>
struct Edge {
  Trigger trigger{Trigger::kAlways};
  GuardFunction<T> guard = [](const T&) -> bool{return true;};
  ActionFunction<T> action = [](T&) -> void{};
  int destination{0};

  // Here, the trigger, guard, and action are ALL optional, and we don't have
  // a more convenient way to do this using C++ (named initializer lists are
  // weirdly C specific) so 2^3 constructors it is!
  Edge(Trigger trigger, GuardFunction<T> guard, ActionFunction<T> action, int destination)
    : trigger{trigger}, guard{guard}, action{action}, destination{destination} {}
  Edge(Trigger trigger, ActionFunction<T> action, int destination)
    : trigger{trigger}, action{action}, destination{destination} {}
  Edge(Trigger trigger, GuardFunction<T> guard, int destination)
    : trigger{trigger}, guard{guard}, destination{destination} {}
  Edge(Trigger trigger, int destination)
    : trigger{trigger}, destination{destination} {}

  Edge(GuardFunction<T> guard, ActionFunction<T> action, int destination)
    : guard{guard}, action{action}, destination{destination} {}
  Edge(ActionFunction<T> action, int destination)
    : action{action}, destination{destination} {}
  Edge(GuardFunction<T> guard, int destination)
    : guard{guard}, destination{destination} {}
  Edge(int destination)
    : destination{destination} {}
};

#define END_OF_EDGES(Type) Edge<Type>{Trigger::kEndOfList, nullptr, nullptr, 0},

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

    void RunLogic(T& state, debug::AiProfiler* profiler = nullptr) {
      std::string current_node_name;
      if (profiler) {
        current_node_name = NodeName(state.current_node);
        profiler->StartState(current_node_name);
      }

      auto current_node = node_list[state.current_node];
      for (auto i = current_node.edge_list; i->trigger != Trigger::kEndOfList; i++) {
      //int edge_index = 0; // for profiling
        /*
        if (profiler) {
          profiler->StartEdge(current_node_name, edge_index);
        }//*/
        auto edge = *i;
        // Make sure we pass this edge's trigger condition
        if (edge.trigger == Trigger::kAlways or edge.trigger == Trigger::kGuardOnly or
            (edge.trigger == Trigger::kFirstFrame and state.frames_at_this_node == 0) or
            (edge.trigger == Trigger::kLastFrame and state.frames_at_this_node >= current_node.duration - 1)) {
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
            /*
            if (profiler) {
              profiler->EndEdge(current_node_name, edge_index);
            }//*/

            //finally, break out so we stop processing edges
            break;
          }
        }
        /*
        if (profiler) {
          profiler->EndEdge(current_node_name, edge_index);
        }
        edge_index++; //*/
      }

      //increment counters, to track actions and lifetimes
      state.frames_alive++;
      state.frames_at_this_node++;

      if (profiler) {
        profiler->EndState(current_node_name);
      }
    }

  private:
    Node<T>* node_list;

};

#endif  // STATE_MACHINE_H_
