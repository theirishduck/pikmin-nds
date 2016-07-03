#include "debug_ui.h"

#include "debug.h"
#include <nds.h>

namespace debug_ui {

void UpdateDebugValues(DebugUiState& debug_ui) {
  debug::UpdateValuesMode();
}

void UpdateDebugTimings(DebugUiState& debug_ui) {
  debug::UpdateTimingMode();
}

void UpdateDebugToggles(DebugUiState& debug_ui) {
  debug::UpdateTogglesMode();
}

void InitDebugSpawners(DebugUiState& debug_ui) {
  debug::InitializeSpawners();
}

void UpdateDebugSpawners(DebugUiState& debug_ui) {
  debug::UpdateSpawnerMode(debug_ui.game);
}

bool DebugSwitcherPressed(const DebugUiState&  debug_ui) {
  return keysDown() & KEY_START;
}

namespace DebugUiNode {
enum DebugUiNode {
  kInit = 0,
  kDebugTimings,
  kDebugValues,
  kDebugToggles,
  kDebugSpawners,
};
}

Edge<DebugUiState> init[] = {
  Edge<DebugUiState>{kAlways, nullptr, nullptr, DebugUiNode::kDebugTimings},
  END_OF_EDGES(DebugUiState)
};

Edge<DebugUiState> debug_timings[] = {
  Edge<DebugUiState>{kAlways, DebugSwitcherPressed, nullptr, DebugUiNode::kDebugValues},
  Edge<DebugUiState>{kAlways, nullptr, UpdateDebugTimings, DebugUiNode::kDebugTimings}, //Loopback
  END_OF_EDGES(DebugUiState)
};

Edge<DebugUiState> debug_values[] = {
  Edge<DebugUiState>{kAlways, DebugSwitcherPressed, nullptr, DebugUiNode::kDebugToggles},
  Edge<DebugUiState>{kAlways, nullptr, UpdateDebugValues, DebugUiNode::kDebugValues}, //Loopback
  END_OF_EDGES(DebugUiState)
};

Edge<DebugUiState> debug_toggles[] = {
  Edge<DebugUiState>{kAlways, DebugSwitcherPressed, InitDebugSpawners, DebugUiNode::kDebugSpawners},
  Edge<DebugUiState>{kAlways, nullptr, UpdateDebugToggles, DebugUiNode::kDebugToggles}, //Loopback
  END_OF_EDGES(DebugUiState)
};

Edge<DebugUiState> debug_spawners[] = {
  Edge<DebugUiState>{kAlways, DebugSwitcherPressed, nullptr, DebugUiNode::kDebugTimings},
  Edge<DebugUiState>{kAlways, nullptr, UpdateDebugSpawners, DebugUiNode::kDebugSpawners}, //Loopback
  END_OF_EDGES(DebugUiState)
};

Node<DebugUiState> node_list[] {
  {"Init", true, init},
  {"Timing", true, debug_timings},
  {"Values", true, debug_values},
  {"Toggles", true, debug_toggles},
  {"Spawners", true, debug_spawners},
};

StateMachine<DebugUiState> machine(node_list);

}  // namespace debug_ui
