#ifndef DEBUG_DRAW_H
#define DEBUG_DRAW_H

#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>

#include "vector.h"

namespace debug {

extern bool g_timing_colors;
extern bool g_render_first_pass_only;
extern bool g_skip_vblank;

void DrawCrosshair(Vec3 p, rgb color = RGB5(31, 31, 31));
void DrawGroundPlane(int width, int segments, rgb color = RGB5(0, 0, 0));
void TimingColor(rgb color);
void UpdateFlags();

}  // namespace debug

#endif  // DEBUG_DRAW_H
