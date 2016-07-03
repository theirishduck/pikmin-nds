#include "debug.h"
#include <cstdio>
#include <nds.h>
#include <map>

#include "pikmin_game.h"
#include "ai/captain.h"

using numeric_types::fixed;
using numeric_types::literals::operator"" _f;

bool debug::g_timing_colors{false};
bool debug::g_render_first_pass_only{false};
bool debug::g_skip_vblank{false};
bool debug::g_physics_circles{false};

void debug::nocashNumber(int num) {
  char buffer[20];
  sprintf(buffer, "%i", num);
  nocashMessage(buffer);
}

void debug::DrawLine(Vec2 p1, Vec2 p2, rgb color) {
  DrawLine(Vec3{p1.x, 0_f, p1.y}, Vec3{p2.x, 0_f, p2.y}, color);
}

void debug::DrawLine(Vec3 p1, Vec3 p2, rgb color) {
  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  glTexParameter(0, 0); //disable textures
  glBegin(GL_TRIANGLE);
  glColor(color);
  glPushMatrix();
  glTranslatef32(p1.x.data_, p1.y.data_ + (1 << 11), p1.z.data_);
  glVertex3v16(0, 0, 0);
  glPopMatrix(1);
  glPushMatrix();
  glTranslatef32(p2.x.data_, p2.y.data_ + (1 << 11), p2.z.data_);
  glVertex3v16(0, 0, 0);
  glVertex3v16(0, 0, 0);
  glPopMatrix(1);
  glEnd();
}

void debug::DrawCrosshair(Vec3 p, rgb color) {
  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  glTexParameter(0, 0); //disable textures
  glBegin(GL_TRIANGLE);
  glColor(color);
  glPushMatrix();
  glTranslatef32(p.x.data_, p.y.data_, p.z.data_);
  glVertex3v16(0, -1 << 10, 0);
  glVertex3v16(0, 1 << 10, 0);
  glVertex3v16(0, 1 << 10, 0);

  glVertex3v16(1 << 10, 0, 0);
  glVertex3v16(-1 << 10, 0, 0);
  glVertex3v16(-1 << 10, 0, 0);

  glPopMatrix(1);
  glEnd();
}

void debug::DrawCircle(Vec3 p, fixed radius, rgb color, u32 segments) {
  float const radiansPerArc = 360.0 / segments;

  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  glTexParameter(0, 0); //disable textures
  glBegin(GL_TRIANGLE);
  glColor(color);
  glPushMatrix();
  // We add 0.5 here to avoid a collision with the ground plane.
  glTranslatef32(p.x.data_, p.y.data_ + (1 << 11), p.z.data_);
  glScalef32(radius.data_,radius.data_,radius.data_);
  //spin right round
  for (u32 i = 0; i < segments; ++i) {
    glPushMatrix();
    glRotateY(i * radiansPerArc);
    glVertex3v16(1 << 12, 0, 0);
    glRotateY(radiansPerArc);
    glVertex3v16(1 << 12, 0, 0);
    glVertex3v16(1 << 12, 0, 0);
    glPopMatrix(1);
  }

  glPopMatrix(1);
  glEnd();

}

void debug::DrawGroundPlane(int width, int segments, rgb color) {
  // Derive a dark color by dividing each channel by 2. This is accomplished
  // using a bitmask: 0 rrrr0 gggg0 bbbb0, which removes the bottom bit in each
  // color channel. Shifting the result of this mask to the right results in
  // 0 0rrrr 0gggg 0bbbb, which is the desired result.
  rgb dark_color = (color & 0x7BDE) >> 1;
  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | (1 << 12));
  glTexParameter(0, 0); //disable textures
  glBegin(GL_TRIANGLE);
  glPushMatrix();
  glScalef(width / 2, 0, width / 2);
  for (int z = 0; z < segments; z++) {
    for (int x = 0; x < segments; x++) {
      glColor((z + x) % 2 == 0 ? color : dark_color);
      glVertex3f(-1.0f + (2.0f / segments) *  x     , 0,  -1.0f + (2.0f / segments) *  z);
      glVertex3f(-1.0f + (2.0f / segments) * (x + 1), 0,  -1.0f + (2.0f / segments) *  z);
      glVertex3f(-1.0f + (2.0f / segments) * (x + 1), 0,  -1.0f + (2.0f / segments) * (z + 1));

      glVertex3f(-1.0f + (2.0f / segments) * (x + 1), 0,  -1.0f + (2.0f / segments) * (z + 1));
      glVertex3f(-1.0f + (2.0f / segments) *  x     , 0,  -1.0f + (2.0f / segments) *  z);
      glVertex3f(-1.0f + (2.0f / segments) *  x     , 0,  -1.0f + (2.0f / segments) * (z + 1));
    }
  }

  glPopMatrix(1);
  glEnd();
}

void debug::_TimingColor(rgb color) {
  if (g_timing_colors) {
    BG_PALETTE_SUB[0] = color;
  }
}

struct TimingResult {
  u32 start = 0;
  u32 end = 0;
  u32 delta() {
    return end - start;
  }
};

struct TopicInfo {
  const char* name;
  rgb color;
};

TimingResult g_timing_results[static_cast<int>(debug::Topic::kNumTopics)];

std::map<debug::Topic, TopicInfo> g_topic_info{
  {debug::Topic::kAI, {
    "AI",
    RGB8(255, 128, 0)}},
  {debug::Topic::kEntityUpdate, {
      "EntityUpdate",
      RGB8(255, 128, 0)}},
  {debug::Topic::kPhysics, {
    "Physics",
    RGB8(255, 64, 255)}},
  {debug::Topic::kFrameInit, {
    "FrameInit",
    RGB8(0, 128, 0)}},
  {debug::Topic::kPassInit, {
    "PassInit",
    RGB8(255, 255, 0)}},
  {debug::Topic::kPass1, {
    "Pass1",
    RGB8(0, 0, 255)}},
  {debug::Topic::kPass2, {
    "Pass2",
    RGB8(0, 0, 255)}},
  {debug::Topic::kPass3, {
    "Pass3",
    RGB8(0, 0, 255)}},
  {debug::Topic::kPass4, {
    "Pass4",
    RGB8(0, 0, 255)}},
  {debug::Topic::kPass5, {
    "Pass5",
    RGB8(0, 0, 255)}},
  {debug::Topic::kPass6, {
    "Pass6",
    RGB8(0, 0, 255)}},
  {debug::Topic::kPass7, {
    "Pass7",
    RGB8(0, 0, 255)}},
  {debug::Topic::kPass8, {
    "Pass8",
    RGB8(0, 0, 255)}},
  {debug::Topic::kPass9, {
    "Pass9",
    RGB8(0, 0, 255)}},
  {debug::Topic::kIdle, {
    "Idle",
    RGB8(48, 48, 48)}},
  {debug::Topic::kUi, {
    "UI",
    RGB8(48, 48, 48)}},
  {debug::Topic::kParticleUpdate, {
    "Particle.Update",
    RGB8(48, 48, 48)}},
  {debug::Topic::kParticleDraw, {
    "Particle.Draw",
    RGB8(48, 48, 48)}},
};

void debug::PrintTitle(const char* title) {
  int console_width = 64;
  int leading_space = (console_width - strlen(title)) / 2 - 1;
  int following_space = leading_space + (strlen(title) % 2);

  printf("%s %s %s", std::string(leading_space, '-').c_str(), title,
    std::string(following_space, '-').c_str());
}

void debug::UpdateTimingMode() {
  // Clear the screen
  printf("\x1b[2J");
  debug::PrintTitle("TIMING");

  // For every topic, output the timing on its own line
  for (int i = 0; i < static_cast<int>(debug::Topic::kNumTopics); i++) {
    if (g_timing_results[i].delta() > 0) {
      if (i & 0x1) {
        printf("\x1b[39m");
      } else {
        printf("\x1b[37m");
      }
      int displayLine = i + 2;
      // Print out the topic name if we have it, otherwise print out something
      // generic
      if (g_topic_info.count((debug::Topic)i)) {
        printf("\x1b[%d;0H%s", displayLine, g_topic_info[(debug::Topic)i].name);
      } else {
        printf("\x1b[%d;0HTopic:%d", displayLine, i);
      }
      printf("\x1b[%d;21H%10lu", displayLine, g_timing_results[i].delta());
    }
  }

  // Reset the colors when we're done
  printf("\x1b[39m");
}

void debug::StartTopic(debug::Topic topic) {
  int index = static_cast<int>(topic);
  g_timing_results[index].start = cpuGetTiming();
  debug::_TimingColor(g_topic_info[topic].color);
}

void debug::EndTopic(debug::Topic topic) {
  int index = static_cast<int>(topic);
  g_timing_results[index].end = cpuGetTiming();
  debug::_TimingColor(RGB8(0,0,0));
}

void debug::ClearTopic(Topic topic) {
  int index = static_cast<int>(topic);
  g_timing_results[index].start = 0;
  g_timing_results[index].end = 0;
}

void debug::StartCpuTimer() {
  // This uses two timers for 32bit precision, so this call will consume
  // timers 0 and 1.
  cpuStartTiming(0);
}

int g_debug_current_topic = 0;

void debug::NextTopic() {
  g_debug_current_topic++;
  if (g_debug_current_topic >= (int)debug::Topic::kNumTopics) {
    g_debug_current_topic = 0;
  }
}

void debug::PreviousTopic() {
  g_debug_current_topic--;
  if (g_debug_current_topic < 0) {
    g_debug_current_topic = (int)debug::Topic::kNumTopics;
  }
}

void debug::UpdateTopic() {
  //clear the line
  printf("\x1b[22;0H                                ");
  if (g_topic_info.count((Topic)g_debug_current_topic)) {
    printf("\x1b[22;0H%s", g_topic_info[(Topic)g_debug_current_topic].name);
  } else {
    printf("\x1b[22;0HTopic:%d", g_debug_current_topic);
  }
  printf("\x1b[22;21H%10lu", g_timing_results[g_debug_current_topic].delta());
}

std::map<std::string, bool*> g_debugToggles {
  {"Skip VBlank", &debug::g_skip_vblank},
  {"Timing Colors", &debug::g_timing_colors},
  {"Only Render First Pass", &debug::g_render_first_pass_only},
  {"Debug Circles", &debug::g_physics_circles},
};

void debug::UpdateTogglesMode() {
  printf("\x1b[2J");
  debug::PrintTitle("Debug Toggles");
  int touch_offset = 16;
  for (auto pair : g_debugToggles) {
    std::string toggleName = pair.first;
    bool* toggleActive = pair.second;
    if (*toggleActive) {
      printf("\x1b[39m");
    } else {
      printf("\x1b[30;1m");
    }
    printf("+------------------------------+\n");
    printf("| (%s) %*s |\n", (*toggleActive ? "*" : " "), 24, toggleName.c_str());
    printf("+------------------------------+\n");

    // figure out if we need to toggle this frame
    if (keysDown() & KEY_TOUCH) {
      touchPosition touch;
      touchRead(&touch);

      if (touch.py > touch_offset and touch.py < touch_offset + 24) {
        *toggleActive = !(*toggleActive);
      }
    }
    touch_offset += 24;
  }

  // Reset the colors when we're done
  printf("\x1b[39m");
}

void debug::AddToggle(std::string name, bool* toggle) {
  g_debugToggles[name] = toggle;
}

namespace {
std::pair<PikminGame::SpawnMap::const_iterator, PikminGame::SpawnMap::const_iterator> g_spawn_names;
PikminGame::SpawnMap::const_iterator g_current_spawner;
}  // namespace

void debug::InitializeSpawners() {
  g_spawn_names = PikminGame::SpawnNames();
  g_current_spawner = g_spawn_names.first;
}

void debug::UpdateSpawnerMode(PikminGame* game) {
  printf("\x1b[2J");
  debug::PrintTitle("Spawn Objects");

  printf("+------+ +-%*s-+ +------+", 42, std::string(42, '-').c_str());
  printf("|      | | %*s | |      |", 42, " ");
  printf("|   <  | | %*s | |  >   |", 42, g_current_spawner->first.c_str());
  printf("|      | | %*s | |      |", 42, " ");
  printf("+------+ +-%*s-+ +------+", 42, std::string(42, '-').c_str());

  if (keysDown() & KEY_TOUCH) {
    touchPosition touch;
    touchRead(&touch);

    if (touch.px > 192) {
      g_current_spawner++;
      if (g_current_spawner == g_spawn_names.second) {
        g_current_spawner = g_spawn_names.first;
      }
    } else if (touch.px < 64) {
      if (g_current_spawner == g_spawn_names.first) {
        g_current_spawner = g_spawn_names.second;
      }
      g_current_spawner--;
    } else {
      //Spawn a thingy!!
      ObjectState* object = game->Spawn(g_current_spawner->first);
      object->set_position(game->ActiveCaptain()->cursor->position());
      object->entity->set_rotation(game->ActiveCaptain()->cursor->rotation());
    }
  }

  // Reset the colors when we're done
  printf("\x1b[39m");
}
