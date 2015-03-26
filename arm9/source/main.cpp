#include <stdio.h>

#include <array>
#include <functional>

#include <nds.h>

#include "multipass_engine.h"
#include "debug.h"
#include "vram_allocator.h"

#include "entities/pellet_posy.h"

#include "ai/pikmin.h"
#include "ai/captain.h"

// Included to debug texture loading.
#include "piki_eyes_img_bin.h"
#include "posy_leaf1_img_bin.h"
#include "posy_leaf2_img_bin.h"
#include "posy_leaf3_img_bin.h"
#include "posy_petal_img_bin.h"
#include "numbers_img_bin.h"

using entities::PelletPosy;

using pikmin_ai::PikminState;
using pikmin_ai::PikminType;

using captain_ai::CaptainState;

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::fixed;

s32 const kTestPikmin{100};

MultipassEngine g_engine;
VramAllocator texture_allocator(VRAM_C, 128 * 1024);

DrawableEntity g_pikmin_entity[kTestPikmin];
PikminState g_pikmin_state[kTestPikmin];
DrawableEntity g_captain;
CaptainState g_captain_state;

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
  for (s32 i = 0; i < kTestPikmin; i += 5) {
    for (s32 j = 0; j < 5; j++) {
      g_pikmin_state[i + j].entity = &g_pikmin_entity[i + j];
      g_pikmin_state[i + j].type = PikminType::kBluePikmin;
      g_pikmin_state[i + j].id = i + j;
      g_engine.AddEntity(&g_pikmin_entity[i + j]);
      g_pikmin_entity[i + j].body()->position = {
        fixed::FromInt(-10 + j * 5),
        0_f,
        fixed::FromInt(-1 - i * -1)};
    }
  }
}

void SetupDemoStage() {
  //spawn in test objects
  PelletPosy* posy = new PelletPosy(texture_allocator);
  g_engine.AddEntity(posy);
  posy->body()->position = {10_f, 0_f, 0_f};
}

void InitCaptain() {
  g_captain_state.entity = &g_captain;
  g_engine.AddEntity(&g_captain);
  g_engine.TargetEntity(&g_captain);
}

void Init() {
  InitMainScreen();
  InitSubScreen();

  printf("\x1b[37mTEST\x1b[39m");
  debug::DisplayValue("Answer", 42);

  LoadTextures();
  SetupDemoPikmin();
  SetupDemoStage();
  InitCaptain();
  

  glPushMatrix();
}

void RunLogic() {
  //TODO: Make this more powerful, handle spawning objects and levels and stuff
  for (int i = 0; i < kTestPikmin; i++) {
    pikmin_ai::machine.RunLogic(g_pikmin_state[i]);
  }

  captain_ai::machine.RunLogic(g_captain_state);
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
    debug::Update();
  }
}

int main() {
  Init();
  GameLoop();
  return 0;
}

