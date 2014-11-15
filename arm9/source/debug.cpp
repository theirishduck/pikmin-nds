#include "debug.h"
#include <cstdio>
#include <nds.h>
#include <map>

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

namespace {
void Status(char const* status) {
  printf("\x1b[23;0H[D] %28.28s", status);
}
}  // namespace

void debug::UpdateInput() {
  // Hold debug modifier [SELECT], then press:
  // A = Render only First Pass
  // B = Skip VBlank, useful for:
  // X = Draw Debug Timings to Bottom Screen (with flashing colors!)

  // Check for debug-related input and update the flags accordingly.
  // Todo(Nick) Make this touchscreen based instead of key combo based.
  if (keysHeld() & KEY_SELECT) {
    if (keysDown() & KEY_A) {
      debug::g_render_first_pass_only = not debug::g_render_first_pass_only;
      if (debug::g_render_first_pass_only) {
        Status("Rendering only first pass.");
      } else {
        Status("Rendering every pass.");
      }
    }

    if (keysDown() & KEY_B) {
      debug::g_skip_vblank = not debug::g_skip_vblank;
      if (debug::g_skip_vblank) {
        Status("Skipping vBlank");
      } else {
        Status("Not skipping vBlank");
      }
    }

    if (keysDown() & KEY_X) {
      debug::g_timing_colors = not debug::g_timing_colors;
      if (debug::g_timing_colors) {
        Status("Rendering Colors");
      } else {
        Status("No more flashing!");
      }
    }

    if (keysDown() & KEY_Y) {
      debug::g_physics_circles = not debug::g_physics_circles;
      if (debug::g_physics_circles) {
        Status("Physics Circles!");
      } else {
        Status("No more circles.");
      }
    }

    //switch timing topics
    if (keysDown() & KEY_LEFT) {
      debug::PreviousTopic();
    }

    if (keysDown() & KEY_RIGHT) {
      debug::NextTopic();
    }
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
  {debug::Topic::kUpdate, {
    "Update", 
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
  {debug::Topic::kDraw, {
    "Draw",   
    RGB8(0, 0, 255)}},
  {debug::Topic::kIdle, {
    "Idle",   
    RGB8(48, 48, 48)}},
};

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
