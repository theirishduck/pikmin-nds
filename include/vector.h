#ifndef VECTOR_H
#define VECTOR_H

#include "fixed.h"

#define Vec3 Vector3<>
#define Vec2 Vector2<>

template <typename T = s32, int F = 12>
struct Vector3 {
    gx::Fixed<T,F> x;
    gx::Fixed<T,F> y;
    gx::Fixed<T,F> z;

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

    Vector3 operator*(const float& other) {
        //multiply the vector elements by a scalar, and return the
        //modified vector
        Vector3 result;
        result.x = x * other;
        result.y = y * other;
        result.z = z * other;
        return result;
    }

    gx::Fixed<T,F> length() {
        //WARNING: Will only function correctly on s32 data types with 12bit fractional components.
        s32 root = sqrtf32((x*x + y*y + z*z).data);
        gx::Fixed<T,F> result;
        result.data = root;
        return result;
    }

    Vector3 normalize() {
        //normalize this vector and return the result
        Vector3 result;
        result.x = x / length();
        result.y = y / length();
        result.z = z / length();
        return result;
    }
};

template <typename T = s32, int F = 12>
struct Vector2 {
    gx::Fixed<T,F> x;
    gx::Fixed<T,F> y;
};

#endif