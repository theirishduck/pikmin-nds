#include "ui.h"

#include <nds.h>
#include <stdio.h>
#include "debug.h"

#include "pikmin_game.h"

#include "bubblefont_img_bin.h"
#include "bubblefont_pal_bin.h"

#include "red_button_light_img_bin.h"
#include "red_button_light_pal_bin.h"
#include "red_button_dark_img_bin.h"
#include "red_button_dark_pal_bin.h"

#include "yellow_button_lit_img_bin.h"
#include "yellow_button_lit_pal_bin.h"
#include "yellow_button_dark_img_bin.h"
#include "yellow_button_dark_pal_bin.h"

#include "blue_button_lit_img_bin.h"
#include "blue_button_lit_pal_bin.h"
#include "blue_button_dark_img_bin.h"
#include "blue_button_dark_pal_bin.h"

using numeric_types::literals::operator"" _f;
using numeric_types::fixed;

using numeric_types::literals::operator"" _brad;
using numeric_types::Brads;

using pikmin_ai::PikminType;

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

void UpdatePikminSelector(UIState& ui, int index, PikminType selected) {
  // TODO: make this not copy data every frame? perhaps?

  // Red pikmin
  oamSetHidden(&oamSub, index, false);
  oamSetXY(&oamSub, index, 0, 0);
  oamSetPalette(&oamSub, index, 1);
  oamSetGfx(&oamSub, index, SpriteSize_64x64, SpriteColorFormat_16Color, 
      ui.game->SpriteAllocator()->Retrieve("redbutton"));
  
  // Yellow pikmin
  oamSetHidden(&oamSub, index + 1, false);
  oamSetXY(&oamSub, index + 1, 0, 64);
  oamSetPalette(&oamSub, index + 1, 2);
  oamSetGfx(&oamSub, index + 1, SpriteSize_64x64, SpriteColorFormat_16Color, 
      ui.game->SpriteAllocator()->Retrieve("yellowbutton"));

  // Blue pikmin
  oamSetHidden(&oamSub, index + 2, false);
  oamSetXY(&oamSub, index + 2, 0, 128);
  oamSetPalette(&oamSub, index + 2, 3);
  oamSetGfx(&oamSub, index + 2, SpriteSize_64x64, SpriteColorFormat_16Color, 
      ui.game->SpriteAllocator()->Retrieve("bluebutton"));

  // Decide which version to display
  auto next_pikmin = ui.game->ActiveCaptain()->squad.NextPikmin();
  if (next_pikmin and next_pikmin->type == PikminType::kRedPikmin) {
    // Use the lit texture
    memcpy(&SPRITE_PALETTE_SUB[16], red_button_light_pal_bin, red_button_light_pal_bin_size);
    ui.game->SpriteAllocator()->Replace("redbutton", red_button_light_img_bin, red_button_light_img_bin_size);
  } else {
    // Use the dark texture
    memcpy(&SPRITE_PALETTE_SUB[16], red_button_dark_pal_bin, red_button_dark_pal_bin_size);
    ui.game->SpriteAllocator()->Replace("redbutton", red_button_dark_img_bin, red_button_dark_img_bin_size);
  }

  if (next_pikmin and next_pikmin->type == PikminType::kYellowPikmin) {
    // Use the lit texture
    memcpy(&SPRITE_PALETTE_SUB[32], yellow_button_lit_pal_bin, yellow_button_lit_pal_bin_size);
    ui.game->SpriteAllocator()->Replace("yellowbutton", yellow_button_lit_img_bin, yellow_button_lit_img_bin_size);
  } else {
    // Use the dark texture
    memcpy(&SPRITE_PALETTE_SUB[32], yellow_button_dark_pal_bin, yellow_button_dark_pal_bin_size);
    ui.game->SpriteAllocator()->Replace("yellowbutton", yellow_button_dark_img_bin, yellow_button_dark_img_bin_size);
  }

  if (next_pikmin and next_pikmin->type == PikminType::kBluePikmin) {
    // Use the lit texture
    memcpy(&SPRITE_PALETTE_SUB[48], blue_button_lit_pal_bin, blue_button_lit_pal_bin_size);
    ui.game->SpriteAllocator()->Replace("bluebutton", blue_button_lit_img_bin, blue_button_lit_img_bin_size);
  } else {
    // Use the dark texture
    memcpy(&SPRITE_PALETTE_SUB[48], blue_button_dark_pal_bin, blue_button_dark_pal_bin_size);
    ui.game->SpriteAllocator()->Replace("bluebutton", blue_button_dark_img_bin, blue_button_dark_img_bin_size);
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
  
  // copy in the pikmin button data
  memcpy(&SPRITE_PALETTE_SUB[16], red_button_light_pal_bin, red_button_light_pal_bin_size);
  ui.game->SpriteAllocator()->Load("redbutton", red_button_light_img_bin, red_button_light_img_bin_size);
  memcpy(&SPRITE_PALETTE_SUB[32], yellow_button_lit_pal_bin, yellow_button_lit_pal_bin_size);
  ui.game->SpriteAllocator()->Load("yellowbutton", yellow_button_lit_img_bin, yellow_button_lit_img_bin_size);
  memcpy(&SPRITE_PALETTE_SUB[48], blue_button_lit_pal_bin, blue_button_lit_pal_bin_size);
  ui.game->SpriteAllocator()->Load("bluebutton", blue_button_lit_img_bin, blue_button_lit_img_bin_size);
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
  UpdatePikminSelector(ui, 109, PikminType::kRedPikmin);
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
