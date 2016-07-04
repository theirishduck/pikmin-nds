#include "debug_ui.h"

#include <cstdio>
#include <functional>
#include <nds.h>

#include "ai/captain.h"
#include "debug/utilities.h"
#include "numeric_types.h"
#include "pikmin_game.h"

using std::string;
using std::function;

using numeric_types::literals::operator"" _brad;

namespace debug_ui {

void InitAlways(DebugUiState& debug_ui) {
  debug_ui.current_spawner = PikminGame::SpawnNames().first;
}

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
  // Clear the screen
  printf("\x1b[2J");
  debug::PrintTitle("TIMING");

  // For every topic, output the timing on its own line
  for (auto topic : debug_ui.game->Engine().DebugProfiler().Topics()) {
    if (topic.timing.delta() > 0) {
      printf("%-32s %31lu", topic.name.c_str(), topic.timing.delta());
    } else {
      printf("\n");
    }
  }

  // Reset the colors when we're done
  printf("\x1b[39m");
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

void UpdateDebugSpawners(DebugUiState& debug_ui) {
  printf("\x1b[2J");
  debug::PrintTitle("Spawn Objects");

  printf("+------+ +-%*s-+ +------+", 42, std::string(42, '-').c_str());
  printf("|      | | %*s | |      |", 42, " ");
  printf("|   <  | | %*s | |  >   |", 42, debug_ui.current_spawner->first.c_str());
  printf("|      | | %*s | |      |", 42, " ");
  printf("+------+ +-%*s-+ +------+", 42, std::string(42, '-').c_str());

  if (keysDown() & KEY_TOUCH) {
    touchPosition touch;
    touchRead(&touch);

    if (touch.px > 192) {
      debug_ui.current_spawner++;
      if (debug_ui.current_spawner == PikminGame::SpawnNames().second) {
        debug_ui.current_spawner = PikminGame::SpawnNames().first;
      }
    } else if (touch.px < 64) {
      if (debug_ui.current_spawner == PikminGame::SpawnNames().first) {
        debug_ui.current_spawner = PikminGame::SpawnNames().second;
      }
      debug_ui.current_spawner--;
    } else {
      //Spawn a thingy!!
      ObjectState* object = debug_ui.game->Spawn(debug_ui.current_spawner->first);
      object->set_position(debug_ui.game->ActiveCaptain()->cursor->position());
      auto object_rotation = debug_ui.game->ActiveCaptain()->cursor->rotation();
      object_rotation.y += 180_brad;
      object->entity->set_rotation(object_rotation);
    }
  }

  // Reset the colors when we're done
  printf("\x1b[39m");
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
  Edge<DebugUiState>{kAlways, nullptr, InitAlways, DebugUiNode::kDebugTimings},
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
  Edge<DebugUiState>{kAlways, DebugSwitcherPressed, nullptr, DebugUiNode::kDebugSpawners},
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
