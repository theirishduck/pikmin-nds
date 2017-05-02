#include "ui.h"

#include <nds.h>
#include <stdio.h>
#include "debug/utilities.h"

#include "pikmin_game.h"
#include "wide_console.h"
#include "ai/captain.h"

// Numbers and fonts
#include "bubblefont_img_bin.h"
#include "bubblefont_pal_bin.h"

// Pikmin buttons (and their states)
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

// Map icons
#include "red_dot_img_bin.h"
#include "yellow_dot_img_bin.h"
#include "blue_dot_img_bin.h"
#include "map_icons_pal_bin.h"

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

void UpdateMapIcons(UIState& ui) {
  // look these up just once
  auto red_dot = ui.game->SpriteAllocator()->Retrieve("red_dot").offset;
  auto yellow_dot = ui.game->SpriteAllocator()->Retrieve("yellow_dot").offset;
  auto blue_dot = ui.game->SpriteAllocator()->Retrieve("blue_dot").offset;

  auto pikmin = ui.game->PikminList();
  auto olimar_position = ui.game->ActiveCaptain()->position();
  for (int slot = 0; slot < 100; slot++) {
    if (pikmin[slot].object.active) {
      auto& current_pikmin = pikmin[slot].object;
      // calculate the on-screen position of these pikmin
      auto position = current_pikmin.position();
      int x = (int)(position.x - olimar_position.x) * 2 + 128;
      int y = (int)(position.z - olimar_position.z) * 2 + 96;

      if (x > 0 and y > 0 and x < 256 and y < 192) {
        // draw the thing!
        oamSetHidden(&oamSub, slot, false);
        oamSetXY(&oamSub, slot, x, y);
        oamSetPalette(&oamSub, slot, 4);
        oamSetPriority(&oamSub, slot, 3);
        switch(current_pikmin.type) {
          case PikminType::kNone:
          case PikminType::kRedPikmin:
            oamSetGfx(&oamSub, slot, SpriteSize_8x8, SpriteColorFormat_16Color,
              red_dot);
            break;
          case PikminType::kYellowPikmin:
          oamSetGfx(&oamSub, slot, SpriteSize_8x8, SpriteColorFormat_16Color,
            yellow_dot);
            break;
          case PikminType::kBluePikmin:
            oamSetGfx(&oamSub, slot, SpriteSize_8x8, SpriteColorFormat_16Color,
              blue_dot);
            break;
        }
      } else {
        oamSetHidden(&oamSub, slot, true);
      }
    } else {
      oamSetHidden(&oamSub, slot, true);
    }
  }
}

void UpdatePikminSelector(UIState& ui, int index) {
  // TODO: make this not copy data every frame? perhaps?

  // Red pikmin
  oamSetHidden(&oamSub, index, false);
  oamSetXY(&oamSub, index, 0, 0);
  oamSetPalette(&oamSub, index, 1);
  oamSetGfx(&oamSub, index, SpriteSize_64x64, SpriteColorFormat_16Color,
      ui.game->SpriteAllocator()->Retrieve("redbutton").offset);

  // Yellow pikmin
  oamSetHidden(&oamSub, index + 1, false);
  oamSetXY(&oamSub, index + 1, 0, 64);
  oamSetPalette(&oamSub, index + 1, 2);
  oamSetGfx(&oamSub, index + 1, SpriteSize_64x64, SpriteColorFormat_16Color,
      ui.game->SpriteAllocator()->Retrieve("yellowbutton").offset);

  // Blue pikmin
  oamSetHidden(&oamSub, index + 2, false);
  oamSetXY(&oamSub, index + 2, 0, 128);
  oamSetPalette(&oamSub, index + 2, 3);
  oamSetGfx(&oamSub, index + 2, SpriteSize_64x64, SpriteColorFormat_16Color,
      ui.game->SpriteAllocator()->Retrieve("bluebutton").offset);

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

  memcpy(&SPRITE_PALETTE_SUB[0], bubblefont_pal_bin, bubblefont_pal_bin_size);
  ui.game->SpriteAllocator()->Replace(
    "bubblefont", bubblefont_img_bin, bubblefont_img_bin_size);
}

void InitDebugScreen(UIState& ui) {
  InitWideConsole();
  ui.debug_screen_active = true;
}

void InitNavPad(UIState& ui) {
  // Set up the video modes for the NavPad
  videoSetModeSub(MODE_2_2D);
  vramSetBankH(VRAM_H_SUB_BG);
  vramSetBankI(VRAM_I_SUB_SPRITE);
  oamInit(&oamSub, SpriteMapping_1D_32, false);

  // Reset the sprite allocator
  ui.game->SpriteAllocator()->Reset();

  // copy in the bubble font data, for number updating
  memcpy(&SPRITE_PALETTE_SUB[0], bubblefont_pal_bin, bubblefont_pal_bin_size);
  ui.game->SpriteAllocator()->Load(
    "bubblefont", bubblefont_img_bin, bubblefont_img_bin_size, {16, 160});

  // copy in the pikmin button data
  memcpy(&SPRITE_PALETTE_SUB[16], red_button_light_pal_bin, red_button_light_pal_bin_size);
  ui.game->SpriteAllocator()->Load(
    "redbutton", red_button_light_img_bin, red_button_light_img_bin_size, {64, 64});
  memcpy(&SPRITE_PALETTE_SUB[32], yellow_button_lit_pal_bin, yellow_button_lit_pal_bin_size);
  ui.game->SpriteAllocator()->Load(
    "yellowbutton", yellow_button_lit_img_bin, yellow_button_lit_img_bin_size, {64, 64});
  memcpy(&SPRITE_PALETTE_SUB[48], blue_button_lit_pal_bin, blue_button_lit_pal_bin_size);
  ui.game->SpriteAllocator()->Load(
    "bluebutton", blue_button_lit_img_bin, blue_button_lit_img_bin_size, {64, 64});

  // setup the map icon data
  memcpy(&SPRITE_PALETTE_SUB[64], map_icons_pal_bin, map_icons_pal_bin_size);
  ui.game->SpriteAllocator()->Load(
    "red_dot", red_dot_img_bin, red_dot_img_bin_size, {8, 8});
  ui.game->SpriteAllocator()->Load(
    "yellow_dot", yellow_dot_img_bin, yellow_dot_img_bin_size, {8, 8});
  ui.game->SpriteAllocator()->Load(
    "blue_dot", blue_dot_img_bin, blue_dot_img_bin_size, {8, 8});
}

void InitAlways(UIState& ui) {
  ui.debug_topic_id = ui.game->Engine().DebugProfiler().RegisterTopic("Game: UI");
  InitNavPad(ui);
}

void UpdateNavPad(UIState& ui) {
  //auto profiler = ui.game->Engine().DebugProfiler();
  ui.game->Engine().DebugProfiler().StartTopic(ui.debug_topic_id);
  // Update pikmin counts
  BubbleNumber(100, 70,  168, ui.game->ActiveCaptain()->squad.squad_size, 3);
  BubbleNumber(103, 114, 168, ui.game->PikminInField(), 3);
  BubbleNumber(106, 158, 168, ui.game->TotalPikmin(), 3);
  UpdatePikminSelector(ui, 109);
  UpdateMapIcons(ui);
  ui.game->Engine().DebugProfiler().EndTopic(ui.debug_topic_id);
}

bool OpenOnionUI(const UIState& ui) {
  if (keysDown() & KEY_A) {
    auto captain = ui.game->ActiveCaptain();
    if (captain->active_onion) {
      return true;
    }
  }
  return false;
}

void InitOnionUI(UIState& ui) {
  // for now, use the console screen for the onion
  InitWideConsole();

  // Pause the main game
  ui.game->PauseGame();

  ui.pikmin_delta = 0;
}

bool key_repeat_active(int frame_timer) {
  const int repeat_delay = 16;
  const int key_delay = 1;

  if (frame_timer == 0) {
    return true;
  }
  if (frame_timer > repeat_delay and frame_timer % key_delay == 0) {
    return true;
  }
  return false;
}

void PauseGame(UIState& ui) {
  ui.game->PauseGame();
  // Todo: something fancier later
  InitWideConsole();
  printf("\n\n\n\n\n\n\n\n\n\n\n");
  printf("          -- PAUSED --");
}

void UnpauseGame(UIState& ui) {
  ui.game->UnpauseGame();
  InitNavPad(ui);
}

bool PauseButtonPressed(const UIState& ui) {
  return (keysDown() & KEY_START);
}

void UpdateOnionUI(UIState& ui) {
  printf("\x1b[2J");

  auto captain = ui.game->ActiveCaptain();
  auto onion = captain->active_onion;
  if (onion->pikmin_type == PikminType::kRedPikmin) {
    printf("Red Pikmin: ");
  }
  if (onion->pikmin_type == PikminType::kYellowPikmin) {
    printf("Yellow Pikmin: ");
  }
  if (onion->pikmin_type == PikminType::kBluePikmin) {
    printf("Blue Pikmin: ");
  }

  printf("Onion UI\n\n");
  int pikmin_in_onion = ui.game->CurrentSaveData()->PikminCount(onion->pikmin_type);
  int pikmin_in_squad = captain->squad.PikminCount(onion->pikmin_type);

  printf("Pikmin in Onion: %d\n", pikmin_in_onion - ui.pikmin_delta);
  if (ui.pikmin_delta >= 0) {
    printf("Withdrawing ");
  } else {
    printf("Depositing ");
  }
  printf("%d\n", abs(ui.pikmin_delta));
  printf("Pikmin in Squad: %d\n", pikmin_in_squad + ui.pikmin_delta);

  if (keysHeld() & (KEY_DOWN | KEY_UP)) {
    if (key_repeat_active(ui.key_timer)) {
      if ((keysHeld() & KEY_UP) and pikmin_in_squad + ui.pikmin_delta > 0) {
        ui.pikmin_delta--;
      }
      if ((keysHeld() & KEY_DOWN) and pikmin_in_onion - ui.pikmin_delta > 0 and ui.game->PikminInField() + ui.pikmin_delta < 100) {
        ui.pikmin_delta++;
      }
    }
    ui.key_timer++;
  } else {
    ui.key_timer = 0;
  }

  if ((keysHeld() & KEY_TOUCH)) {
    if (key_repeat_active(ui.touch_timer)) {
      touchPosition touch;
      touchRead(&touch);

      if (touch.py < 64 and pikmin_in_squad + ui.pikmin_delta > 0) {
        ui.pikmin_delta--;
      }
      if (touch.py > 128 and pikmin_in_onion - ui.pikmin_delta > 0 and ui.game->PikminInField() + ui.pikmin_delta <= 100) {
        ui.pikmin_delta++;
      }
    }
    ui.touch_timer++;
  } else {
    ui.touch_timer = 0;
  }
}

bool CloseOnionUI(const UIState& ui) {
  return (keysDown() & KEY_A);
}

bool CancelOnionUI(const UIState& ui) {
  return (keysDown() & KEY_B);
}

void ApplyOnionDelta(UIState& ui) {
  if (ui.pikmin_delta > 0) {
    ui.game->ActiveCaptain()->active_onion->withdraw_count = ui.pikmin_delta;
  }
  if (ui.pikmin_delta < 0) {
    //TODO: Handle this using pikmin states instead of removing them here
    auto captain = ui.game->ActiveCaptain();
    auto active_onion = captain->active_onion;
    int squad_index = 0;
    while (squad_index < captain->squad.squad_size and ui.pikmin_delta < 0) {
      if (captain->squad.pikmin[squad_index] != nullptr and
          captain->squad.pikmin[squad_index]->type == active_onion->pikmin_type) {
        auto pikmin = captain->squad.pikmin[squad_index];
        // Have this pikmin randomly target one of the onion's feet, and
        // set its collision group accordingly
        pikmin->entity->body_handle().body->sensor_groups = ONION_FEET_GROUP;
        Vec3 onion_foot_position = captain->active_onion->feet[rand() % 3]->position;
        pikmin->target = Vec2{onion_foot_position.x, onion_foot_position.z};
        pikmin->has_target = true;

        // Remove the pikmin from the captain's squad
        captain->squad.RemovePikmin(pikmin);

        ui.pikmin_delta++;
      } else {
        squad_index++;
      }
    }
  }

  ui.game->UnpauseGame();
}

bool DebugButtonPressed(const UIState&  ui) {
  return keysDown() & KEY_SELECT;
}

void UpdateDebugScreen(UIState& ui) {
  //printf("Update Debug Screen!\n");
  debug_ui::machine.RunLogic(ui.debug_state);
  ui.game->DebugDictionary().Set("Debug Active: ", (int)ui.debug_screen_active);
}

bool DebugScreenActive(const UIState& ui) {
  return ui.debug_screen_active;
}

void CloseDebugScreen(UIState& ui) {
  ui.debug_screen_active = 0;
  InitNavPad(ui);
}

namespace UINode {
enum UINode {
  kSleep = 0,
  kInit,
  kNavPad,
  kOnionUI,
  kOnionClosing,
  kDebugScreen,
  kPauseScreen,
};
}

Edge<UIState> wait_frame[] = {
  Edge<UIState>{Trigger::kAlways, nullptr, nullptr, UINode::kInit},
  END_OF_EDGES(UIState)
};

Edge<UIState> init[] = {
  Edge<UIState>{Trigger::kAlways, nullptr, InitAlways, UINode::kNavPad},
  END_OF_EDGES(UIState)
};

Edge<UIState> nav_pad[] = {
  Edge<UIState>{Trigger::kAlways, DebugButtonPressed, InitDebugScreen, UINode::kDebugScreen},
  Edge<UIState>{Trigger::kAlways, OpenOnionUI, InitOnionUI, UINode::kOnionUI},
  Edge<UIState>{Trigger::kAlways, PauseButtonPressed, PauseGame, UINode::kPauseScreen},
  Edge<UIState>{Trigger::kAlways, nullptr, UpdateNavPad, UINode::kNavPad}, //Loopback
  END_OF_EDGES(UIState)
};

Edge<UIState> onion_ui[] = {
  Edge<UIState>{Trigger::kAlways, CloseOnionUI, ApplyOnionDelta, UINode::kOnionClosing},
  Edge<UIState>{Trigger::kAlways, CancelOnionUI, UnpauseGame, UINode::kOnionClosing},
  Edge<UIState>{Trigger::kAlways, nullptr, UpdateOnionUI, UINode::kOnionUI},
  END_OF_EDGES(UIState)
};

Edge<UIState> closing_onion_ui[] = {
  Edge<UIState>{Trigger::kAlways, DebugScreenActive, InitDebugScreen, UINode::kDebugScreen},
  Edge<UIState>{Trigger::kAlways, nullptr, InitNavPad, UINode::kNavPad},
  END_OF_EDGES(UIState)
};

Edge<UIState> pause_screen[] = {
  Edge<UIState>{Trigger::kAlways, PauseButtonPressed, UnpauseGame, UINode::kNavPad},
  END_OF_EDGES(UIState)
};

Edge<UIState> debug_screen[] = {
  Edge<UIState>{Trigger::kAlways, DebugButtonPressed, CloseDebugScreen, UINode::kNavPad},
  Edge<UIState>{Trigger::kAlways, OpenOnionUI, InitOnionUI, UINode::kOnionUI},
  Edge<UIState>{Trigger::kAlways, nullptr, UpdateDebugScreen, UINode::kDebugScreen}, // Loopback
};

Node<UIState> node_list[] {
  {"Sleep", true, wait_frame},
  {"Init", true, init},
  {"NavPad", true, nav_pad},
  {"OnionUI", true, onion_ui},
  {"OnionClosing", true, closing_onion_ui},
  {"DebugScreen", true, debug_screen},
  {"PauseScreen", true, pause_screen},
};

StateMachine<UIState> machine(node_list);

}  // namespace ui
