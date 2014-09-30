#ifndef BASIC_MECHANICS_H
#define BASIC_MECHANICS_H

#include <nds/arm9/videoGL.h>

constexpr v16 operator"" _v16(long double value) {
  return static_cast<v16>(value * 4096);
}

constexpr v16 operator"" _v16(unsigned long long value) {
  return static_cast<v16>(value * 4096);
}

constexpr int32 operator"" _f32(long double value) {
  return static_cast<int32>(value * 4096);
}

constexpr int32 operator"" _f32(unsigned long long value) {
  return static_cast<int32>(value * 4096);
}

constexpr int32 int32FromFloat(float value) {
  return static_cast<int32>(value * 4096);
}

constexpr float floatFromInt32(int32 value) {
  return static_cast<float>(value) / 4096;
}

constexpr v16 v16FromFloat(float value) {
  return static_cast<v16>(value * 4096);
}

void basicMechanicsUpdate();
void basicMechanicsDraw();

#endif  // BASIC_MECHANICS_H
