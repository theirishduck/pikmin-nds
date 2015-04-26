#include <stdio.h>

#include <array>
#include <functional>

#include <nds.h>

#include "multipass_engine.h"
#include "pikmin_game.h"
#include "debug.h"

#include "entities/pellet_posy.h"
#include "entities/level.h"

#include "ai/pikmin.h"
#include "ai/captain.h"
#include "ai/onion.h"

// Included to debug texture loading.
#include "piki_eyes_img_bin.h"
#include "piki_leaf_img_bin.h"
#include "posy_leaf1_img_bin.h"
#include "posy_leaf2_img_bin.h"
#include "posy_leaf3_img_bin.h"
#include "posy_petal_img_bin.h"
#include "numbers_img_bin.h"
#include "rocky_img_bin.h"
#include "checkerboard_img_bin.h"
#include "cursor_img_bin.h"
#include "bad_whistle_img_bin.h"
#include "flower_img_bin.h"
#include "redonion_img_bin.h"

// Level data and heightmaps
#include "checkerboard_height_bin.h"

using entities::PelletPosy;
using entities::Level;

using pikmin_ai::PikminState;
using pikmin_ai::PikminType;

using captain_ai::CaptainState;

using onion_ai::OnionState;

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::fixed;

using debug::Topic;

s32 const kTestPikmin{100};

MultipassEngine g_engine;
PikminGame g_game(g_engine);

// Initialize the console using the full version of the console init function so
// that VRAM bank H can be used instead of the default bank, bank C.
void InitDebugConsole() {
  vramSetBankH(VRAM_H_SUB_BG);
  videoSetModeSub(MODE_0_2D);

  PrintConsole* const kDefaultConsole{nullptr};
  s32 const kConsoleLayer{0};
  s32 const kConsoleMapBase{15};
  s32 const kConsoleTileBase{0};
  bool const kConsoleOnMainDisplay{true};
  bool const kLoadConsoleGraphics{true};
  consoleInit(kDefaultConsole, kConsoleLayer, BgType_Text4bpp, BgSize_T_256x256,
      kConsoleMapBase, kConsoleTileBase, not kConsoleOnMainDisplay,
      kLoadConsoleGraphics);
}

void InitSubScreen() {
  InitDebugConsole();
}

void InitMainScreen() {
  videoSetMode(MODE_0_3D);
  glInit();
  glEnable(GL_TEXTURE_2D | GL_BLEND);

  glClearColor(4, 4, 4, 31);
  glClearDepth(0x7FFF);
  glViewport(0, 0, 255, 191);

  // Setup default lights; these will be overridden in the main engine.
  glLight(0, RGB15(31, 31, 31), floattov10(-0.40), floattov10(0.32),
      floattov10(0.27));
  glLight(1, RGB15(31, 31, 31), floattov10(0.32), floattov10(0.32),
      floattov10(0.32));
}

void LoadTextures() {
  // VRAM is not memory mapped to the CPU when in texture mode, so all
  // modifications to textures must be done by changing the bank to a mode
  // where it is mapped to the CPU, performing the modifications, and
  // switching it back to texture mode.
  vramSetBankC(VRAM_C_LCD);

  g_game.TextureAllocator()->Load("piki_eyes", piki_eyes_img_bin, piki_eyes_img_bin_size);
  g_game.TextureAllocator()->Load("piki_leaf", piki_leaf_img_bin, piki_leaf_img_bin_size);
  g_game.TextureAllocator()->Load("posy-leaf1", posy_leaf1_img_bin, posy_leaf1_img_bin_size);
  g_game.TextureAllocator()->Load("posy-leaf2", posy_leaf2_img_bin, posy_leaf2_img_bin_size);
  g_game.TextureAllocator()->Load("posy-leaf3", posy_leaf3_img_bin, posy_leaf3_img_bin_size);
  g_game.TextureAllocator()->Load("posy-petal", posy_petal_img_bin, posy_petal_img_bin_size);
  g_game.TextureAllocator()->Load("numbers", numbers_img_bin, numbers_img_bin_size);
  g_game.TextureAllocator()->Load("rocky", rocky_img_bin, rocky_img_bin_size);
  g_game.TextureAllocator()->Load("cursor", cursor_img_bin, cursor_img_bin_size);
  g_game.TextureAllocator()->Load("checkerboard", checkerboard_img_bin, checkerboard_img_bin_size);
  g_game.TextureAllocator()->Load("bad_whistle", bad_whistle_img_bin, bad_whistle_img_bin_size);
  g_game.TextureAllocator()->Load("flower", flower_img_bin, flower_img_bin_size);
  g_game.TextureAllocator()->Load("redonion", redonion_img_bin, redonion_img_bin_size);
  
  vramSetBankC(VRAM_C_TEXTURE);
}

void SetupDemoPikmin() {
  for (s32 i = 0; i < kTestPikmin; i += 10) {
    for (s32 j = 0; j < 10; j++) {
      //*
      PikminState* pikmin = g_game.SpawnObject<PikminState>();
      //random colors!
      pikmin->type = (PikminType)((rand() % 3) + 1);
      //pikmin->type = PikminType::kRedPikmin;

      pikmin->entity->body()->position = {
        fixed::FromInt(-10 + j * 1 + 64),
        0_f,
        fixed::FromInt(-1 - (i * -0.2) - 64)};
    }
  }
}

void SetupDemoStage() {
  //spawn in test objects
  PelletPosy* posy = new PelletPosy(g_game.TextureAllocator());
  g_engine.AddEntity(posy);
  posy->body()->position = {6.2_f, 0_f, -6.2_f};

  //load in the test level
  Level* sandbox = new Level(g_game.TextureAllocator());
  g_engine.AddEntity(sandbox);
  g_engine.World().SetHeightmap(checkerboard_height_bin);

  //spawn in an onion!
  auto onion = g_game.SpawnObject<OnionState>();
  onion->entity->body()->position = Vec3{64_f, 0_f, -72_f};
}

void InitCaptain() {
  CaptainState* captain = g_game.SpawnObject<CaptainState>();
  g_engine.TargetEntity(captain->entity);
  captain->entity->body()->position = Vec3{64_f,0_f,-62_f};

}

void Init() {
  InitMainScreen();
  InitSubScreen();

  LoadTextures();
  SetupDemoPikmin();
  InitCaptain();
  SetupDemoStage();
  
  glPushMatrix();
}

void RunLogic() {
  g_game.Step();
}

void GameLoop() {
  for (;;) {
    touchPosition touchXY;
    touchRead(&touchXY);

    //start debug timings for this loop
    debug::StartCpuTimer();

    RunLogic();

    g_engine.Update();
    g_engine.Draw();
    //debug::Update();

    oamUpdate(&oamSub);
  }
}

int main() {
  Init();
  GameLoop();
  return 0;
}
