#ifndef DEBUG_DRAW_H
#define DEBUG_DRAW_H

#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>

#include "vector.h"
#include "numeric_types.h"

namespace debug {

extern bool g_timing_colors;
extern bool g_render_first_pass_only;
extern bool g_skip_vblank;
extern bool g_physics_circles;

//some quick functions to make nocash debugging a bit easier
void nocashNumber(int num);

void DrawCrosshair(Vec3 p, rgb color = RGB5(31, 31, 31));
void DrawGroundPlane(int width, int segments, rgb color = RGB5(0, 0, 0));
void DrawCircle(Vec3 p, numeric_types::fixed radius, rgb color, u32 segments = 8);
void _TimingColor(rgb color);
void UpdateInput();

enum Topic {
  kPhysics = 0,
  kUpdate,
  kFrameInit,
  kPassInit,
  kDraw,
  kIdle,
  kNumTopics
};

void StartTopic(Topic topic);
void EndTopic(Topic topic);
void StartCpuTimer();
void NextTopic();
void PreviousTopic();
void UpdateTopic();

}  // namespace debug

#endif  // DEBUG_DRAW_H
