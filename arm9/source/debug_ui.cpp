#include "debug_ui.h"

#include <cstdio>
#include <functional>
#include <nds.h>

#include "debug.h"
#include "pikmin_game.h"

using std::string;
using std::function;

namespace debug_ui {

void UpdateDebugValues(DebugUiState& debug_ui) {
  // Clear the screen
  printf("\x1b[2J");
  debug::PrintTitle("VALUES");

  int display_position = 2;
  auto display_values = debug_ui.game->DebugDictionary().DisplayValues();
  for (auto kv : display_values) {
    if (display_position < 22) {
      printf("\x1b[39m%s: \x1b[36;1m%s\n", kv.first.c_str(), kv.second.c_str());
      display_position++;
    }
  }

  // Reset the colors when we're done
  printf("\x1b[39m");
}

void UpdateDebugTimings(DebugUiState& debug_ui) {
  debug::UpdateTimingMode();
}

void UpdateDebugToggles(DebugUiState& debug_ui) {
  printf("\x1b[2J");
  debug::PrintTitle("Debug Toggles");
  int touch_offset = 16;
  auto &debug_flags = debug_ui.game->Engine().debug_flags;
  for (auto pair : debug_flags) {
    std::string toggleName = pair.first;
    bool toggleActive = pair.second;
    if (toggleActive) {
      printf("\x1b[39m");
    } else {
      printf("\x1b[30;1m");
    }
    printf("+------------------------------+\n");
    printf("| (%s) %*s |\n", (toggleActive ? "*" : " "), 24, toggleName.c_str());
    printf("+------------------------------+\n");

    // figure out if we need to toggle this frame
    if (keysDown() & KEY_TOUCH) {
      touchPosition touch;
      touchRead(&touch);

      if (touch.py > touch_offset and touch.py < touch_offset + 24) {
        //*toggleActive = !(*toggleActive);
        debug_flags[toggleName] = !toggleActive;
      }
    }
    touch_offset += 24;
  }

  // Reset the colors when we're done
  printf("\x1b[39m");
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
