#include <stdio.h>

#include <array>
#include <functional>

#include <nds.h>

#include "basic_mechanics.h"
#include "multipass_engine.h"
#include "debug.h"
#include "vram_allocator.h"

#include "entities/captain.h"
#include "entities/pikmin.h"
#include "entities/pellet_posy.h"

// Included to debug texture loading.
#include "piki_eyes_img_bin.h"
#include "posy_leaf1_img_bin.h"
#include "posy_leaf2_img_bin.h"
#include "posy_leaf3_img_bin.h"
#include "posy_petal_img_bin.h"
#include "numbers_img_bin.h"

using entities::Pikmin;
using entities::PikminType;
using entities::Captain;
using entities::PelletPosy;

using numeric_types::literals::operator"" _f;
using numeric_types::fixed;

s32 const kTestPikmin{3};

MultipassEngine g_engine;
VramAllocator texture_allocator(VRAM_C, 128 * 1024);

Pikmin g_red_pikmin[kTestPikmin];
Pikmin g_yellow_pikmin[kTestPikmin];
Pikmin g_blue_pikmin[kTestPikmin];
Captain g_captain;

// Initialize the console using the full version of the console init function so
// that VRAM bank H can be used instead of the default bank, bank C.
void InitSubScreen() {
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

void InitDebugConsole() {
  consoleDebugInit(DebugDevice_NOCASH);
}

void InitMainScreen() {
  videoSetMode(MODE_0_3D);
  glInit();
  glEnable(GL_TEXTURE_2D | GL_BLEND);

  glClearColor(4, 4, 4, 31);
  // TODO(Nick?) Play with this - it may be why there used to be clipping at
  // the back plane.
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

  //dmaCopy(piki_eyes_img_bin, VRAM_C, piki_eyes_img_bin_size);
  texture_allocator.Load("piki_eyes", piki_eyes_img_bin, piki_eyes_img_bin_size);
  texture_allocator.Load("posy-leaf1", posy_leaf1_img_bin, posy_leaf1_img_bin_size);
  texture_allocator.Load("posy-leaf2", posy_leaf2_img_bin, posy_leaf2_img_bin_size);
  texture_allocator.Load("posy-leaf3", posy_leaf2_img_bin, posy_leaf3_img_bin_size);
  texture_allocator.Load("posy-petal", posy_petal_img_bin, posy_petal_img_bin_size);
  texture_allocator.Load("numbers", numbers_img_bin, numbers_img_bin_size);
  
  vramSetBankC(VRAM_C_TEXTURE);
}

void SetupDemoPikmin() {
  for (s32 i = 0; i < kTestPikmin; i++) {
    g_red_pikmin[i].SetPikminType(PikminType::kRedPikmin);
    g_red_pikmin[i].set_position({-5_f, 0_f,
        fixed::FromInt(-2 + i * -2)});
    g_engine.AddEntity(&g_red_pikmin[i]);

    g_yellow_pikmin[i].SetPikminType(PikminType::kYellowPikmin);
    g_yellow_pikmin[i].set_position({0_f, 0_f,
        fixed::FromInt(-2 + i * -2)});
    g_engine.AddEntity(&g_yellow_pikmin[i]);

    g_blue_pikmin[i].SetPikminType(PikminType::kBluePikmin);
    g_blue_pikmin[i].set_position({5_f, 0_f,
        fixed::FromInt(-2 + i * -2)});
    g_engine.AddEntity(&g_blue_pikmin[i]);
  }
}

void SetupDemoStage() {
  //spawn in test objects
  PelletPosy* posy = new PelletPosy(texture_allocator);
  posy->set_position({10_f, 0_f, 0_f});
  g_engine.AddEntity(posy);
}

void InitCaptain() {
  g_captain.set_position({0_f, 0_f, 0_f});
  g_captain.SetAnimation("Armature|Idle1");
  g_engine.AddEntity(&g_captain);
  g_engine.TargetEntity(&g_captain);
}

void Init() {
  InitSubScreen();
  InitDebugConsole();
  printf("Multipass Engine Demo\n");

  InitMainScreen();

  LoadTextures();
  SetupDemoPikmin();
  SetupDemoStage();
  InitCaptain();
  

  glPushMatrix();
}

void GameLoop() {
  for (;;) {
    touchPosition touchXY;
    touchRead(&touchXY);

    //start debug timings for this loop
    debug::StartCpuTimer();
    debug::UpdateTopic();

    basicMechanicsUpdate();

    g_engine.Update();
    g_engine.Draw();
  }
}

int main() {
  Init();
  GameLoop();
  return 0;
}

