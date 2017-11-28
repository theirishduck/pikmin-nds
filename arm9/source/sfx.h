#ifndef SFX_H
#define SFX_H

#include "numeric_types.h"
#include <maxmod9.h>

namespace sfx {

struct Effect {
	mm_word sample_id{0};
	numeric_types::Fixed<s32,12> pitch_variation{numeric_types::Fixed<s32,12>::FromInt(0)};
};

enum sfx {
  kFootstep,
  kTotal
};

void PlaySound(int effect_id);

} // namespace sfx

#endif