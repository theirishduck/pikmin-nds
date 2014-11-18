#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

namespace StateMachine {

enum Trigger {
  kAlways = 0,
  kEveryFrame,
  kFirstFrame, //ie, framecount for the state == 0
  kLastAnimFrame, //framecount for the state == animation length
};

struct Node;

struct Edge {
  Trigger trigger;
  GuardFunction guard;
  ActionFunction action;
  Node* destination;
};

struct Node {
  bool can_rest;
  Edge* begin_edge;
  Edge* end_edge;
  char* animation;
};

class ObjectState {
  public:
    ObjectState();
    ~ObjectState();
};

}  // StateMachine

#endif  // STATE_MACHINE_H_