#include "treasure.h"

#include "dsgx.h"

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;
using numeric_types::fixed;

namespace treasure_ai {

Edge<TreasureState> noop[] {
  END_OF_EDGES(TreasureState)
};

Node<TreasureState> node_list[] {
  {"noop", true, noop},
};

StateMachine<TreasureState> machine(node_list);

}  // namespace treasure_ai
