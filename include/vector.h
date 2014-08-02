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
};

template <typename T = s32, int F = 12>
struct Vector2 {
    gx::Fixed<T,F> x;
    gx::Fixed<T,F> y;
};

#endif