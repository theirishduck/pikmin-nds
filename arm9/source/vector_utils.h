#ifndef VECTOR_UTILS_H
#define VECTOR_UTILS_H

#include "numeric_types.h"
#include "vector.h"

inline numeric_types::Brads AngleFromVec2(Vec2 direction) {
  if (direction.Length().data_ > 0) {
    direction = direction.Normalize();
    if (direction.y.data_ <= 0) {
      return numeric_types::Brads::Raw(acosLerp(direction.x.data_));
    } else {
      return numeric_types::Brads::Raw(-acosLerp(direction.x.data_));
    }
  }
  return numeric_types::Brads::Raw(acosLerp(0));
}

inline numeric_types::Brads AngleFromNormalizedVec2(Vec2 direction) {
  if (direction.y.data_ <= 0) {
    return numeric_types::Brads::Raw(acosLerp(direction.x.data_));
  } else {
    return numeric_types::Brads::Raw(-acosLerp(direction.x.data_));
  }
}

#endif
