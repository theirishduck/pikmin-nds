#include <nds.h>
#include <stdio.h>

// Console Fonts
#include "yesh1_stretched_256_img_bin.h"
#include "yesh1_stretched_256_pal_bin.h"

// Initialize the console using the full version of the console init function so
// that VRAM bank H can be used instead of the default bank, bank C.

u16 _color_blend(u16 rgb5_a, u16 rgb5_b) {
  // unpack
  u16 Ar =  rgb5_a & 0x001F;
  u16 Ag = (rgb5_a & 0x03E0) >> 5;
  u16 Ab = (rgb5_a & 0x7A00) >> 10;

  u16 Br =  rgb5_b & 0x001F;
  u16 Bg = (rgb5_b & 0x03E0) >> 5;
  u16 Bb = (rgb5_b & 0x7A00) >> 10;

  // Blend
  u16 Fr = (((Ar + 1) * (Br + 1)) - 1) / 32;
  u16 Fg = (((Ag + 1) * (Bg + 1)) - 1) / 32;
  u16 Fb = (((Ab + 1) * (Bb + 1)) - 1) / 32;

  return RGB5(Fr, Fg, Fb);
}

PrintConsole wide_console;
void InitWideConsole() {
  swiWaitForVBlank();
  vramSetBankH(VRAM_H_SUB_BG);
  videoSetModeSub(MODE_5_2D);

  ConsoleFont YeshFont;
  YeshFont.asciiOffset = ' ';
  YeshFont.bpp = 4;
  YeshFont.convertSingleColor = false;
  YeshFont.gfx = (u16*)yesh1_stretched_256_img_bin;
  YeshFont.pal = (u16*)yesh1_stretched_256_pal_bin;
  YeshFont.numChars = 96 * 2;
  YeshFont.numColors = 16;

  s32 const kConsoleLayer{3};
  s32 const kConsoleMapBase{12};
  s32 const kConsoleTileBase{0};
  bool const kConsoleOnMainDisplay{true};
  bool const kLoadConsoleGraphics{true};
  consoleInit(&wide_console, kConsoleLayer, BgType_ExRotation, BgSize_ER_512x512,
      kConsoleMapBase, kConsoleTileBase, not kConsoleOnMainDisplay,
      not kLoadConsoleGraphics);
  consoleSetWindow(&wide_console, 0, 0, 64, 24);
  wide_console.consoleWidth = 64;

  swiWaitForVBlank();
  bgSetScale(wide_console.bgId, 1 << 9, 1 << 8);
  bgUpdate();

  consoleSetFont(&wide_console, &YeshFont);

  // Now we need to duplicate the font palette and colorize the entries
  u16 font_colors[] = {
    // Dark Colors
    RGB5( 4, 4, 4), // black
    RGB5(16, 8, 8), // red
    RGB5( 8,16, 8), // green
    RGB5(16,16, 8), // yellow
    RGB5( 8, 8,16), // blue
    RGB5(16, 8,16), // magenta
    RGB5( 8,16,16), // cyan
    RGB5(16,16,16), // white

    // "Bright" Colors
    RGB5( 8, 8, 8), // black
    RGB5(31,16,16), // red
    RGB5(16,31,16), // green
    RGB5(31,31,16), // yellow
    RGB5(16,16,31), // blue
    RGB5(31,16,31), // magenta
    RGB5(16,31,31), // cyan
    RGB5(31,31,31), // white
  };

  //for (int font_color = 0; font_color < 16; font_color++) {
    u16* font_palette = (u16*)yesh1_stretched_256_pal_bin;
    for (int palette_color = 0; palette_color < 256; palette_color++) {
      BG_PALETTE_SUB[palette_color] = font_palette[palette_color];
    }
  //}

  // Go ahead and clear the console out for safety
  printf("\x1b[2J");
}
