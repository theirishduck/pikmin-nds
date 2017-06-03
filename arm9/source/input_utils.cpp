#include "input_utils.h"
#include "nds.h"

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

Brads input::DPadDirection()  {
  // Todo(Nick) This feels messy. Find a way to make this cleaner.

  if (keysHeld() & KEY_RIGHT) {
    if (keysHeld() & KEY_UP) {
      return 45_brad;
    }
    if (keysHeld() & KEY_DOWN) {
      return 315_brad;
    }
    return 0_brad;
  }

  if (keysHeld() & KEY_LEFT) {
    if (keysHeld() & KEY_UP) {
      return 135_brad;
    }
    if (keysHeld() & KEY_DOWN) {
      return 225_brad;
    }
    return 180_brad;
  }

  if (keysHeld() & KEY_UP) {
    return 90_brad;
  }

  if (keysHeld() & KEY_DOWN) {
    return 270_brad;
  }

  return 0_brad;
}
