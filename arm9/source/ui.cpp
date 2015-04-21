#include "ui.h"

#include <nds.h>
#include <stdio.h>
#include "debug.h"

#include "pikmin_game.h"

#include "bubblefont_img_bin.h"
#include "bubblefont_pal_bin.h"

using numeric_types::literals::operator"" _f;
using numeric_types::fixed;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

namespace ui {

//Note here: the xy coordinate is for the RIGHTMOST digit
void BubbleNumber(int index, int x, int y, int value, int cells) {
  for (int i = 0; i < cells; i++) {
    if (value > 0 or i == 0) {
      int digit = value % 10;
      value /= 10;

      oamSetHidden(&oamSub, index, false);
      oamSetXY(&oamSub, index, x, y);
      oamSetGfx(&oamSub, index, SpriteSize_16x16, SpriteColorFormat_16Color, 
          oamGetGfxPtr(&oamSub, 0 + digit * 4));
    } else {
      oamSetHidden(&oamSub, index, true);
    }
    index++;
    x -= 12;
  }
}

void InitNavPad(UIState& ui) {
  // Set up the video modes for the NavPad
  vramSetBankH(VRAM_H_SUB_BG);
  vramSetBankI(VRAM_I_SUB_SPRITE);
  videoSetModeSub(MODE_2_2D);
  oamInit(&oamSub, SpriteMapping_1D_32, false);

  // copy in the bubble font data, for number updating
  memcpy(&SPRITE_PALETTE_SUB[0], bubblefont_pal_bin, bubblefont_pal_bin_size);
  ui.game->SpriteAllocator()->Load("bubblefont", bubblefont_img_bin, bubblefont_img_bin_size);
  
}

// Initialize the console using the full version of the console init function so
// that VRAM bank H can be used instead of the default bank, bank C.
void InitDebug(UIState& ui) {
  vramSetBankH(VRAM_H_SUB_BG);
  videoSetModeSub(MODE_2_2D);

  PrintConsole* const kDefaultConsole{nullptr};
  s32 const kConsoleLayer{0};
  s32 const kConsoleMapBase{15};
  s32 const kConsoleTileBase{0};
  bool const kConsoleOnMainDisplay{true};
  bool const kLoadConsoleGraphics{true};
  consoleInit(kDefaultConsole, kConsoleLayer, BgType_Text4bpp, BgSize_T_256x256,
      kConsoleMapBase, kConsoleTileBase, not kConsoleOnMainDisplay,
      kLoadConsoleGraphics);

  // Because we have no idea what state our console is going to be in after
  // the game has been running for a bit, go ahead and clear it
  printf("\x1b[2J");
}

void UpdateNavPad(UIState& ui) {
  // Update pikmin counts
  BubbleNumber(100, 70,  168, ui.game->ActiveCaptain()->squad.squad_size, 3);
  BubbleNumber(103, 114, 168, ui.game->PikminInField(), 3);
  BubbleNumber(106, 158, 168, ui.game->PikminInField(), 3);
}

void UpdateDebugValues(UIState& ui) {
  debug::UpdateValuesMode();
}

void UpdateDebugTimers(UIState& ui) {
  debug::UpdateTimingMode();
}

bool DebugButtonPressed(const UIState&  ui) {
  return keysDown() & KEY_SELECT;
}

namespace UINode {
enum UINode {
  kInit = 0,
  kNavPad,
  kDebugTiming,
  kDebugValues,
};
}

Edge<UIState> edge_list[] {
  // Init
  Edge<UIState>{kAlways, nullptr, InitNavPad, UINode::kNavPad},

  // NavPad
  Edge<UIState>{kAlways, DebugButtonPressed, InitDebug, UINode::kDebugTiming},
  Edge<UIState>{kAlways, nullptr, UpdateNavPad, UINode::kNavPad}, //Loopback

  // Debug Timings
  Edge<UIState>{kAlways, DebugButtonPressed, nullptr, UINode::kDebugValues},
  Edge<UIState>{kAlways, nullptr, UpdateDebugTimers, UINode::kDebugTiming}, //Loopback

  // Debug Values
  Edge<UIState>{kAlways, DebugButtonPressed, InitNavPad, UINode::kNavPad},
  Edge<UIState>{kAlways, nullptr, UpdateDebugValues, UINode::kDebugValues}, //Loopback

};

Node node_list[] {
  {"Init", true, 0, 0},
  {"NavPad", true, 1, 2},
  {"DebugTiming", true, 3, 4},
  {"DebugValues", true, 5, 6},
};

StateMachine<UIState> machine(node_list, edge_list);

}  // namespace ui
