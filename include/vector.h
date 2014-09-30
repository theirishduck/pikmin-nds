#ifndef VECTOR_H
#define VECTOR_H

#include <nds/ndstypes.h>

#include "fixed.h"

template <typename T = s32, int F = 12>
struct Vector3 {
  template <typename FixedT, int FixedF>
  using Fixed = numeric_types::Fixed<FixedT, FixedF>;

  Fixed<T, F> x;
  Fixed<T, F> y;
  Fixed<T, F> z;

  Vector3 operator+(const Vector3& other) {
    Vector3 sum;
    sum.x = x + other.x;
    sum.y = y + other.y;
    sum.z = z + other.z;
    return sum;
  }

  Vector3 operator-(const Vector3& other) {
    Vector3 difference;
    difference.x = x - other.x;
    difference.y = y - other.y;
    difference.z = z - other.z;
    return difference;
  }

  // Multiply the vector elements by a scalar and return the new vector.
  Vector3 operator*(const float& other) {
    Vector3 result;
    result.x = x * other;
    result.y = y * other;
    result.z = z * other;
    return result;
  }

  // Warning: Only correct for 1.19.12 fixed specialization.
  Fixed<T, F> length() {
    s32 root = sqrtf32((x * x + y * y + z * z).data);
    Fixed<T, F> result;
    result.data = root;
    return result;
  }

  // Return a unit vector with the same orientation as this instance.
  Vector3<T, F> normalize() {
    Vector3<T, F> result;
    Fixed<T, F> current_length = length();
    if (current_length == 0) {
        return Vector3{0, 0, 0};
    }
    result.x.data = divf32(x.data, current_length.data);
    result.y.data = divf32(y.data, current_length.data);
    result.z.data = divf32(z.data, current_length.data);
    return result;
  }
};

template <typename T = s32, int F = 12>
struct Vector2 {
  template <typename FixedT, int FixedF>
  using Fixed = numeric_types::Fixed<FixedT, FixedF>;

  Fixed<T, F> x;
  Fixed<T, F> y;
};

using Vec3 = Vector3<>;
using Vec2 = Vector2<>;

#endif  // VECTOR_H
