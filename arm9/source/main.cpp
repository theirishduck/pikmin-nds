#include <array>
#include <functional>
#include <stdio.h>
#include <dirent.h>
#include <set>

#include <filesystem.h>
#include <nds.h>

#include "ai/captain.h"
#include "ai/onion.h"
#include "ai/fire_spout.h"
#include "ai/treasure.h"
#include "ai/pellet_posy.h"
#include "debug/messages.h"
#include "debug/utilities.h"
#include "render/multipass_renderer.h"
#include "particle_library.h"
#include "pikmin_game.h"

// Level data and heightmaps
#include "checkerboard_height_bin.h"

using captain_ai::CaptainState;

using onion_ai::OnionState;
using posy_ai::PosyState;
using pikmin_ai::PikminType;
using fire_spout_ai::FireSpoutState;
using treasure_ai::TreasureState;

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::fixed;

using debug::Topic;

using namespace std;

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
  glEnable(GL_TEXTURE_2D | GL_BLEND | GL_OUTLINE/* | GL_FOG*/);

  // Fog!
  glFogColor(4, 4, 4, 31);
  glFogOffset(0);
  glFogShift(5);
  for (u32 i = 0; i < 32; i++) {
    glFogDensity(i, i * 4);
  }

  glClearColor(4, 4, 4, 31);
  glClearDepth(0x7FFF);
  glViewport(0, 0, 255, 191);

  GFX_CLEAR_COLOR = GFX_CLEAR_COLOR | POLY_FOG;

  Vec3 light0_direction = Vec3{1_f, -1_f, -1_f}.Normalize() * 0.99_f;
  Vec3 light1_direction = Vec3{-1_f, -1_f, -1_f}.Normalize() * 0.99_f;

  // Setup default lights; these will be overridden in the main engine.
  //*
  glLight(0, RGB15(31, 31, 31),
    floattov10((float)light0_direction.x),
    floattov10((float)light0_direction.y),
    floattov10((float)light0_direction.z));
  //*/
  //*
  glLight(1, RGB15(31, 31, 31),
    floattov10((float)light1_direction.x),
    floattov10((float)light1_direction.y),
    floattov10((float)light1_direction.z));
  //*/
  /*
  glLight(2, RGB15(0, 0, 0),
    floattov10((float)light1_direction.x),
    floattov10((float)light1_direction.y),
    floattov10((float)light1_direction.z));
  //*/
  /*
  glLight(3, RGB15(0, 0, 0),
      floattov10((float)light1_direction.x),
      floattov10((float)light1_direction.y),
      floattov10((float)light1_direction.z));
  //*/

  //ds uses a table for shinyness..this generates a half-ass one
	glMaterialShinyness();
}

vector<string> FilesInDirectory(string path) {
  auto dir = opendir(path.c_str());
  vector<string> directories;
  if (dir) {
    while (auto file_entry = readdir(dir)) {
      if (strcmp(".", file_entry->d_name) == 0 or strcmp("..", file_entry->d_name) == 0) {
        continue;
      }
      if (file_entry->d_type == DT_DIR) {
        continue;
      }
      directories.push_back(file_entry->d_name);
    }
  }
  return directories;
}

string BaseName(string full_filename) {
  auto index = full_filename.rfind(".");
  if (index != string::npos) {
    return full_filename.substr(0, index);
  } else {
    return "";
  }
}

string FileExtension(string filename) {
  auto index = filename.rfind(".");
  if (index != string::npos) {
    return filename.substr(index + 1);
  } else {
    return "";
  }
}

map<string, u32> texture_extension_formats = {
  {"2bpp", GL_RGB4},
  {"t2bpp", GL_RGB4},
  {"4bpp", GL_RGB16},
  {"t4bpp", GL_RGB16},
  {"a3i5", GL_RGB32_A3},
  {"a5i3", GL_RGB8_A5},
};

set<string> texture_extension_is_transparent {
  "t2bpp",
  "t4bpp",
};
template<typename T>
void LoadFileWithMetadata(T* vram_allocator, typename T::Metadata metadata, string filename, string identifier) {
  // Load the file data into a temporary buffer
  auto file = fopen(("/textures/" + filename).c_str(), "rb");
  if (file) {
    fseek(file, 0, SEEK_END);
    auto const size = ftell(file);
    fseek(file, 0, SEEK_SET);

    vector<char> buffer(size);
    if (fread(buffer.data(), 1, size, file)) {
      vram_allocator->Load(
        identifier, (u8*)buffer.data(), size, metadata);
    } else {
      debug::Log("NitroFS Read FAILED for");
      debug::Log(filename.c_str());
    }
    fclose(file);
  } else {
    debug::Log("NitroFS Open FAILED for");
    debug::Log(filename.c_str());
  }
}

void LoadDsgxFile(DsgxAllocator* dsgx_allocator, string filename, string identifier) {
  // Load the file data into a temporary buffer
  auto file = fopen(("/actors/" + filename).c_str(), "rb");
  if (file) {
    fseek(file, 0, SEEK_END);
    auto const size = ftell(file);
    fseek(file, 0, SEEK_SET);

    vector<char> buffer(size);
    if (fread(buffer.data(), 1, size, file)) {
      dsgx_allocator->Load(
        identifier, (u8*)buffer.data(), size);
    } else {
      debug::Log("NitroFS Read FAILED for");
      debug::Log(filename.c_str());
    }
    fclose(file);
  } else {
    debug::Log("NitroFS Open FAILED for");
    debug::Log(filename.c_str());
  }
}

void LoadTexturesFromNitroFS(PikminGame& game) {
  auto texture_files = FilesInDirectory("/textures");
  for (string filename : texture_files) {
    // Collect metadata about this file
    auto extension = FileExtension(filename);
    Texture metadata;
    if (texture_extension_formats.count(extension) > 0) {
      metadata.format = texture_extension_formats[extension];
    } else {
      //debug::Log(("Skipping extension: " + extension).c_str());
      continue;
    }
    if (texture_extension_is_transparent.count(extension) > 0) {
      metadata.transparency = Texture::kTransparent;
    } else {
      metadata.transparency = Texture::kDisplayed;
    }

    // Load the file data into a temporary buffer
    LoadFileWithMetadata(game.TextureAllocator(), metadata, filename, filename);

    // Attempt to find and load a matching palette file, if one exists
    string const palette_filename = BaseName(filename) + ".pal";
    LoadFileWithMetadata(game.TexturePaletteAllocator(), TexturePalette{}, palette_filename, filename);
  }
}

void LoadActors(PikminGame& game) {
  auto texture_files = FilesInDirectory("/actors");
  for (string filename : texture_files) {
    auto extension = FileExtension(filename);
    if (extension == "dsgx") {
      // load and parse the DSGX data
      LoadDsgxFile(game.ActorAllocator(), filename, BaseName(filename));
      // apply texture offsets from our previously loaded textures and palettes
      Dsgx* actor = game.ActorAllocator()->Retrieve(BaseName(filename));
      actor->ApplyTextures(game.TextureAllocator(), game.TexturePaletteAllocator());
    }
  }
}

void LoadTextures(PikminGame& game) {
  // VRAM is not memory mapped to the CPU when in texture mode, so all
  // modifications to textures must be done by changing the bank to a mode
  // where it is mapped to the CPU, performing the modifications, and
  // switching it back to texture mode.
  vramSetBankC(VRAM_C_LCD);
  vramSetBankG(VRAM_G_LCD);
  LoadTexturesFromNitroFS(game);
  vramSetBankC(VRAM_C_TEXTURE);
  vramSetBankG(VRAM_G_TEX_PALETTE);
}

void SetupDemoStage(PikminGame& game) {
  //load in the test level
  Drawable* sandbox = new Drawable();
  sandbox->set_actor(game.ActorAllocator()->Retrieve("checker_test"));
  game.renderer().AddEntity(sandbox);
  game.world().SetHeightmap(checkerboard_height_bin);

  //spawn in an onion!
  auto red_onion = game.RetrieveOnion(game.Spawn("Onion:Red"));
  if (red_onion) { red_onion->set_position(Vec3{64_f, 0_f, -32_f}); }

  //spawn in a yellow onion too!
  auto yellow_onion = game.RetrieveOnion(game.Spawn("Onion:Yellow"));
  if (yellow_onion) { yellow_onion->set_position(Vec3{96_f, 0_f, -64_f}); }

  //spawn in a blue onion while we're at it
  auto blue_onion = game.RetrieveOnion(game.Spawn("Onion:Blue"));
  if (blue_onion) { blue_onion->set_position(Vec3{64_f, 0_f, -96_f}); }

  //auto posy = g_game.SpawnObject<PosyState>();
  auto posy = game.RetrievePelletPosy(game.Spawn("Enemy:PelletPosy"));
  if (posy) { posy->set_position(Vec3{44_f, 0_f, -72_f}); }

  auto fire_spout = game.RetrieveFireSpout(game.Spawn("Hazard:FireSpout"));
  if (fire_spout) { fire_spout->set_position(Vec3{64_f, 0_f, -64_f}); }

  auto pellet = game.RetrieveTreasure(game.Spawn("Corpse:Pellet:Red"));
  if (pellet) { pellet->set_position(Vec3{54_f, 0_f, -64_f}); }
}

void InitCaptain(PikminGame& game) {
  CaptainState* captain = game.RetrieveCaptain(game.SpawnCaptain());
  if (captain) {
    game.camera().follow_captain = captain->handle;
    captain->set_position(Vec3{64_f,0_f,-62_f});
  }
}

void Init(PikminGame& game) {
  // filesystem testing stuff
  if (nitroFSInit(NULL)) {
    //debug::Log("Filesystem SUCCESS");
  } else {
    debug::Log("Filesystem FAILURE");
  }

  InitMainScreen();
  InitSubScreen();

  LoadTextures(game);
  LoadActors(game);
  particle_library::Init(game.TextureAllocator(), game.TexturePaletteAllocator());
  InitCaptain(game);
  SetupDemoStage(game);

  glPushMatrix();
}

//returns a random vector from -1 to 1 in all directions
Vec3 RandomVector() {
  return Vec3{
    fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12)),
    fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12)),
    fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12))
  };
}

void GameLoop(PikminGame& game) {
  for (;;) {
    touchPosition touchXY;
    touchRead(&touchXY);

    //start debug timings for this loop
    debug::Profiler::StartTimer();

    game.Step();
    game.renderer().Draw();

    oamUpdate(&oamSub);
  }
}

int main() {
  MultipassRenderer* renderer = new MultipassRenderer();
  PikminGame* game = new PikminGame(*renderer);

  debug::Log("Hello World!");
  Init(*game);
  GameLoop(*game);
  return 0;
}
