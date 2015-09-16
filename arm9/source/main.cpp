#include <stdio.h>
#include <dirent.h>

#include <array>
#include <functional>
#include <set>

#include <nds.h>
#include <filesystem.h>

#include "multipass_engine.h"
#include "pikmin_game.h"
#include "particle.h"
#include "debug.h"

#include "entities/level.h"

#include "ai/pikmin.h"
#include "ai/captain.h"
#include "ai/onion.h"

// Level data and heightmaps
#include "checkerboard_height_bin.h"

using entities::Level;

using pikmin_ai::PikminState;
using pikmin_ai::PikminType;

using captain_ai::CaptainState;

using onion_ai::OnionState;
using posy_ai::PosyState;

using numeric_types::literals::operator"" _f;
using numeric_types::literals::operator"" _brad;
using numeric_types::fixed;

using debug::Topic;

using namespace std;

s32 const kTestPikmin{0};

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
  glEnable(GL_TEXTURE_2D | GL_BLEND | GL_OUTLINE);

  glClearColor(4, 4, 4, 31);
  glClearDepth(0x7FFF);
  glViewport(0, 0, 255, 191);

  // Setup default lights; these will be overridden in the main engine.
  glLight(0, RGB15(31, 31, 31), floattov10(-0.40), floattov10(0.32),
      floattov10(0.27));
  glLight(1, RGB15(31, 31, 31), floattov10(0.32), floattov10(0.32),
      floattov10(0.32));
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
};

set<string> texture_extension_is_transparent {
  "t2bpp",
  "t4bpp",
};
template<typename T>
void LoadFile(T* vram_allocator, typename T::Metadata metadata, string filename, string identifier) {
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
      nocashMessage(("NitroFS LOADED: '" + filename + "'").c_str());
      debug::nocashNumber(size);
    } else {
      nocashMessage("NitroFS Read FAILED for");
      nocashMessage(filename.c_str());
    }
    fclose(file);
  } else {
    nocashMessage("NitroFS Open FAILED for");
    nocashMessage(filename.c_str());
  }
}

void LoadTexturesFromNitroFS() {
  auto texture_files = FilesInDirectory("/textures");
  for (string filename : texture_files) {
    // Collect metadata about this file
    auto extension = FileExtension(filename);
    Texture metadata;
    if (texture_extension_formats.count(extension) > 0) {
      metadata.format = texture_extension_formats[extension];
    } else {
      nocashMessage(("Skipping extension: " + extension).c_str());
      continue;
    }
    if (texture_extension_is_transparent.count(extension) > 0) {
      metadata.transparency = Texture::kTransparent;
    } else {
      metadata.transparency = Texture::kDisplayed;
    }

    // Load the file data into a temporary buffer
    LoadFile(g_game.TextureAllocator(), metadata, filename, filename);

    // Attempt to find and load a matching palette file, if one exists
    string const palette_filename = BaseName(filename) + ".pal";
    LoadFile(g_game.TexturePaletteAllocator(), TexturePalette{}, palette_filename, filename);
  }
}

void LoadTextures() {
  // VRAM is not memory mapped to the CPU when in texture mode, so all
  // modifications to textures must be done by changing the bank to a mode
  // where it is mapped to the CPU, performing the modifications, and
  // switching it back to texture mode.
  vramSetBankC(VRAM_C_LCD);
  vramSetBankG(VRAM_G_LCD);
  LoadTexturesFromNitroFS();
  vramSetBankC(VRAM_C_TEXTURE);
  vramSetBankG(VRAM_G_TEX_PALETTE);
}

void SetupDemoPikmin() {
  static std::string pikmin_names[] = {"Pikmin:Red", "Pikmin:Yellow", "Pikmin:Blue"};
  for (s32 i = 0; i < kTestPikmin; i += 10) {
    for (s32 j = 0; j < min(kTestPikmin, (s32)10); j++) {
      //random colors!
      auto pikmin = g_game.Spawn<PikminState>(pikmin_names[rand() % 3]);

      pikmin->entity->body()->position = {
        fixed::FromInt(-10 + j * 1 + 64),
        0_f,
        fixed::FromInt(-1 - (i * -0.2) - 64)};
    }
  }
}

void SetupDemoStage() {
  //spawn in test objects
  /*PelletPosy* posy = new PelletPosy(g_game.TextureAllocator(), g_game.TexturePaletteAllocator());
  g_engine.AddEntity(posy);
  posy->body()->position = {6.2_f, 0_f, -6.2_f};*/

  //load in the test level
  Level* sandbox = new Level(g_game.TextureAllocator(), g_game.TexturePaletteAllocator());
  g_engine.AddEntity(sandbox);
  g_engine.World().SetHeightmap(checkerboard_height_bin);

  //spawn in an onion!
  auto red_onion = g_game.SpawnObject<OnionState>();
  red_onion->entity->body()->position = Vec3{64_f, 0_f, -32_f};
  red_onion->pikmin_type = PikminType::kRedPikmin;

  //spawn in a yellow onion too!
  auto yellow_onion = g_game.SpawnObject<OnionState>();
  yellow_onion->entity->body()->position = Vec3{96_f, 0_f, -64_f};
  yellow_onion->pikmin_type = PikminType::kYellowPikmin;

  //spawn in a blue onion while we're at it
  auto blue_onion = g_game.SpawnObject<OnionState>();
  blue_onion->entity->body()->position = Vec3{64_f, 0_f, -96_f};
  blue_onion->pikmin_type = PikminType::kBluePikmin;


  //auto posy = g_game.SpawnObject<PosyState>();
  auto posy = g_game.Spawn<PosyState>("Enemy:PelletPosy");
  posy->entity->body()->position = Vec3{44_f, 0_f, -72_f};
}

void InitCaptain() {
  CaptainState* captain = g_game.SpawnObject<CaptainState>();
  g_engine.camera()->FollowCaptain(captain);
  captain->entity->body()->position = Vec3{64_f,0_f,-62_f};

}

void Init() {
  // filesystem testing stuff
  if (nitroFSInit(NULL)) {
    nocashMessage("Filesystem SUCCESS");
  } else {
    nocashMessage("Filesystem FAILURE");
  }

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

//returns a random vector from -1 to 1 in all directions
Vec3 RandomVector() {
  return Vec3{
    fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12)),
    fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12)),
    fixed::FromRaw((rand() & ((1 << 13) - 1)) - (1 << 12))
  };
}

void GameLoop() {
  Particle test_flower;
  test_flower.texture = g_game.TextureAllocator()->Retrieve("fire.a3i5");
  test_flower.palette = g_game.TexturePaletteAllocator()->Retrieve("fire.a3i5");
  test_flower.position = Vec3{64_f, 10_f, -64_f};
  test_flower.lifespan = 64;
  test_flower.fade_rate = 1_f / 64_f;

  int frame_counter = 0;
  for (;;) {
    frame_counter++;
    touchPosition touchXY;
    touchRead(&touchXY);

    //start debug timings for this loop
    debug::StartCpuTimer();

    if ((frame_counter & 0x7) == 0) {
      Particle* new_particle = SpawnParticle(test_flower);
      new_particle->velocity = RandomVector() * 0.005_f + Vec3{0_f,0.02_f,0_f};
    }

    debug::DisplayValue("Particles: ", ActiveParticles());

    if (frame_counter % 2 == 0) {
      RunLogic();
    } else {
      g_engine.Update();
    }
    g_engine.Draw();

    oamUpdate(&oamSub);
  }
}

int main() {
  Init();
  GameLoop();
  return 0;
}
