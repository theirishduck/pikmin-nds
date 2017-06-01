#ifndef VECTOR_H
#define VECTOR_H

#include <nds/ndstypes.h>

#include "numeric_types.h"
#include "trig.h"

template <typename T = s32, int F = 12>
struct Vector3 {
  template <typename FixedT, int FixedF>
  using Fixed = numeric_types::Fixed<FixedT, FixedF>;

  Fixed<T, F> x;
  Fixed<T, F> y;
  Fixed<T, F> z;

  Vector3 operator+(const Vector3& other) const {
    Vector3 sum;
    sum.x = x + other.x;
    sum.y = y + other.y;
    sum.z = z + other.z;
    return sum;
  }

  Vector3& operator+=(const Vector3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  Vector3 operator-(const Vector3& other) const {
    Vector3 difference;
    difference.x = x - other.x;
    difference.y = y - other.y;
    difference.z = z - other.z;
    return difference;
  }

  // Multiply the vector elements by a scalar and return the new vector.
  Vector3 operator*(const Fixed<s32,12>& other) const {
    Vector3 result;
    result.x = x * other;
    result.y = y * other;
    result.z = z * other;
    return result;
  }

  Vector3& operator*=(const Fixed<s32,12>& other) {
    x *= other;
    y *= other;
    z *= other;
    return *this;
  }

  bool operator==(const Vector3& other) {
    return (x == other.x and y == other.y and z == other.z);
  }

  // Warning: Only correct for 1.19.12 fixed specialization.
  Fixed<T, F> Length() const {
    s32 root = sqrtf32((x * x + y * y + z * z).data_);
    Fixed<T, F> result;
    result.data_ = root;
    return result;
  }

  //Returns the length of the vector without the sqrt. This is useful for
  //comparing against squared length for faster checks.
  Fixed<T, F> Length2() const {
    return x*x + y*y + z*z;
  }

  // Return a unit vector with the same orientation as this instance.
  Vector3<T, F> Normalize() const {
    Vector3<T, F> result;
    Fixed<T, F> current_length = Length();
    if (current_length == Fixed<s32,12>::FromInt(0)) {
        return Vector3{Fixed<s32,12>::FromInt(0), Fixed<s32,12>::FromInt(0), Fixed<s32,12>::FromInt(0)};
    }
    result.x.data_ = divf32(x.data_, current_length.data_);
    result.y.data_ = divf32(y.data_, current_length.data_);
    result.z.data_ = divf32(z.data_, current_length.data_);
    return result;
  }
};

template <typename T = s32, int F = 12>
struct Vector2 {
  template <typename FixedT, int FixedF>
  using Fixed = numeric_types::Fixed<FixedT, FixedF>;

  // Warning: Only correct for 1.19.12 fixed specialization.
  Fixed<T, F> Length() const {
    s32 root = sqrtf32((x * x + y * y).data_);
    Fixed<T, F> result;
    result.data_ = root;
    return result;
  }

  Fixed<T, F> Length2() const {
    return x*x + y*y;
  }

  // Return a unit vector with the same orientation as this instance.
  Vector2<T, F> Normalize() const {
    Vector2<T, F> result;
    Fixed<T, F> current_length = Length();
    if (current_length == Fixed<s32,12>::FromInt(0)) {
        return Vector2{Fixed<s32,12>::FromInt(0), Fixed<s32,12>::FromInt(0)};
    }
    result.x.data_ = divf32(x.data_, current_length.data_);
    result.y.data_ = divf32(y.data_, current_length.data_);
    return result;
  }

  Vector2<T, F> Rotate(numeric_types::Brads angle) const {
    return Vector2<T, F>{
      x * trig::CosLerp(angle) - y * trig::SinLerp(angle),
      Fixed<s32,12>::FromInt(-1) * x * trig::SinLerp(angle) - y * trig::CosLerp(angle),
    };
  }

  Vector2 operator+(const Vector2& other) const {
    Vector2 sum;
    sum.x = x + other.x;
    sum.y = y + other.y;
    return sum;
  }

  Vector2& operator+=(const Vector2& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  Vector2 operator-(const Vector2& other) const {
    Vector2 difference;
    difference.x = x - other.x;
    difference.y = y - other.y;
    return difference;
  }

  // Multiply the vector elements by a scalar and return the new vector.
  Vector2 operator*(const Fixed<s32,12>& other) const {
    Vector2 result;
    result.x = x * other;
    result.y = y * other;
    return result;
  }

  Vector2& operator*=(const Fixed<s32,12>& other) {
    x *= other;
    y *= other;
    return *this;
  }

  Fixed<T, F> x;
  Fixed<T, F> y;
};

using Vec3 = Vector3<>;
using Vec2 = Vector2<>;

#endif  // VECTOR_H
