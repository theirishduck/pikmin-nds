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
};

template <typename T = s32, int F = 12>
struct Vector2 {
    gx::Fixed<T,F> x;
    gx::Fixed<T,F> y;
};

#endif