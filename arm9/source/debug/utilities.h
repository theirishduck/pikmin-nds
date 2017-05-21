#ifndef DEBUG_UTILITIES_H
#define DEBUG_UTILITIES_H

#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>

#include "vector.h"
#include "numeric_types.h"
#include <string>
#include <sstream>

class PikminGame;

namespace debug {

template <typename T>
void nocashValue(std::string name, T value) {
  std::ostringstream oss;
  oss << name << ": " << value;
  nocashMessage(oss.str().c_str());
}

std::string to_string(int number);

void DrawCrosshair(Vec3 p, rgb color = RGB5(31, 31, 31));
void DrawGroundPlane(int width, int segments, rgb color = RGB5(0, 0, 0));
void DrawCircle(Vec3 p, numeric_types::fixed radius, rgb color, u32 segments = 8);
void DrawLine(Vec3 p1, Vec3 p2, rgb color);
void DrawLine(Vec2 p1, Vec2 p2, rgb color);
void _TimingColor(rgb color);
void Update();
void UpdateTogglesMode();
void AddToggle(std::string name, bool* toggle);
}  // namespace debug

#endif  // DEBUG_UTILITIES_H
