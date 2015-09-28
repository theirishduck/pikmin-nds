#include "static.h"

#include "dsgx.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

namespace static_ai {

Edge<StaticState> noop[] {
  END_OF_EDGES(StaticState)
};

Node<StaticState> node_list[] {
  {"noop", true, noop},
};

StateMachine<StaticState> machine(node_list);

}  // namespace static_ai
