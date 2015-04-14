#ifndef TRIG_H
#define TRIG_H

#include "numeric_types.h"
#include <nds.h>

namespace trig {

static inline numeric_types::fixed CosLerp(numeric_types::Brads angle)  {
  return numeric_types::fixed::FromRaw(cosLerp(angle.data_));
}

static inline numeric_types::fixed SinLerp(numeric_types::Brads angle)  {
  return numeric_types::fixed::FromRaw(sinLerp(angle.data_));
}

static inline numeric_types::fixed TanLerp(numeric_types::Brads angle)  {
  return numeric_types::fixed::FromRaw(tanLerp(angle.data_));
}

static inline numeric_types::fixed ASinLerp(numeric_types::Brads angle)  {
  return numeric_types::fixed::FromRaw(asinLerp(angle.data_));
}

static inline numeric_types::fixed ACosLerp(numeric_types::Brads angle)  {
  return numeric_types::fixed::FromRaw(acosLerp(angle.data_));
}

}  // namespace trig

#endif
