#ifndef BASICMECHANICS_H
#define BASICMECHANICS_H

#include <nds.h>

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

#endif  // BASICMECHANICS_H
