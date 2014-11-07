#include "numeric_types.h"

using numeric_types::fixed;
using numeric_types::Fixed;

// Hardware division for the 20.12 case; this should be quicker in almost all cases
/*
template<>
fixed Fixed<s32, 12>::operator/(const fixed& other) {fixed r; r.data_ = divf32(data_, other.data_); return r;}
template<>
fixed& Fixed<s32, 12>::operator/=(const fixed& other) {data_ = divf32(data_, other.data_); return *this;}
*/