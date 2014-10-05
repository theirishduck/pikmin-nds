#include <stdio.h>

#include <array>
#include <functional>

#include <nds.h>

#include "basic_mechanics.h"
#include "captain.h"
#include "multipass_engine.h"
#include "red_pikmin.h"
#include "yellow_pikmin.h"

// Included to debug texture loading.
#include "piki_eyes_img_bin.h"

using numeric_types::literals::operator"" _f;
using numeric_types::fixed;

u32 const kTestPikmin{33};

MultipassEngine g_engine;
RedPikmin g_red_pikmin[kTestPikmin];
RedPikmin g_red_pikmin2[kTestPikmin];
RedPikmin g_red_pikmin3[kTestPikmin];
YellowPikmin g_yellow_pikmin[kTestPikmin];
Captain g_captain[kTestPikmin];
Captain g_captain2[kTestPikmin];
Captain g_captain3[kTestPikmin];

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
  glEnable(GL_TEXTURE_2D);

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

void SetupDemoPikmin() {
  for (u32 i = 0; i < kTestPikmin; i++) {
    g_red_pikmin[i].set_position({-5_f, 0_f,
        fixed::FromInt(-2 + static_cast<s32>(i) * -2)});
    g_engine.AddEntity(&g_red_pikmin[i]);

    g_red_pikmin2[i].set_position({0_f, 0_f,
        fixed::FromInt(-2 + static_cast<s32>(i) * -2)});
    g_engine.AddEntity(&g_red_pikmin2[i]);

    g_red_pikmin3[i].set_position({5_f, 0_f,
        fixed::FromInt(-2 + static_cast<s32>(i) * -2)});
    g_engine.AddEntity(&g_red_pikmin3[i]);
  }
}

void InitCaptain() {
  g_captain[0].set_position({0_f, 1_f, 0_f});
  g_captain[0].SetAnimation("Armature|Idle1");
  g_engine.AddEntity(&g_captain[0]);
  g_engine.TargetEntity(&g_captain[0]);
}

// Copy the pikmin eye texture into the beginning of VRAM bank C.
void InitPikminEyeTexture() {
  // VRAM is not memory mapped to the CPU when in texture mode, so all
  // modifications to textures must be done by changing the bank to a mode
  // where it is mapped to the CPU, performing the modifications, and
  // switching it back to texture mode.
  vramSetBankC(VRAM_C_LCD);
  dmaCopy(piki_eyes_img_bin, VRAM_C, piki_eyes_img_bin_size);
  vramSetBankC(VRAM_C_TEXTURE);
}

void Init() {
  InitSubScreen();
  InitDebugConsole();
  printf("Multipass Engine Demo\n");

  InitMainScreen();

  SetupDemoPikmin();
  InitCaptain();
  InitPikminEyeTexture();

  glPushMatrix();
}

void GameLoop() {
  for (;;) {
    touchPosition touchXY;
    touchRead(&touchXY);

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
