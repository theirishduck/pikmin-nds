#ifndef NUMERICTYPES_H
#define NUMERICTYPES_H

#include <nds.h>

#include "fixed.h"

namespace numeric_types {

template <typename T, int F>
using Fixed = gx::Fixed<T, F>;

class Degrees
{
public:
  using value_type = s32;

  Degrees() : data{0} {}
  Degrees(Degrees const& other) : data{other.data} {}
  explicit constexpr Degrees(value_type const value) : data{value} {}
  Degrees& operator=(Degrees const other) {data = other.data; return *this;}

  constexpr bool operator==(Degrees const other) const {return data == other.data;}
  constexpr bool operator<(Degrees const other) const {return data < other.data;}
  constexpr bool operator>(Degrees const other) const {return data > other.data;}
  constexpr bool operator<=(Degrees const other) const {return data <= other.data;}
  constexpr bool operator>=(Degrees const other) const {return data >= other.data;}
  constexpr bool operator!=(Degrees const other) const {return data != other.data;}

  constexpr Degrees operator+(Degrees const other) const {return Degrees{data + other.data};}
  Degrees& operator+=(Degrees const other) {data += other.data; return *this;}
  constexpr Degrees operator-(Degrees const other) const {return Degrees{data - other.data};}
  Degrees& operator-=(Degrees const other) {data -= other.data; return *this;}
  constexpr Degrees operator*(Degrees const other) const {return Degrees{data * other.data};}
  Degrees& operator*=(Degrees const other) {data *= other.data; return *this;}
  Degrees const operator/(Degrees const other) const {return Degrees{div32(data, other.data)};}
  Degrees& operator/=(Degrees const other) {data = div32(data, other.data); return *this;}
  Degrees const operator%(Degrees const other) const {return Degrees{mod32(data, other.data)};}
  Degrees& operator%=(Degrees const other) {data = mod32(data, other.data); return *this;}

  constexpr Degrees div(Degrees const other) const {return Degrees{data / other.data};}
  constexpr Degrees mod(Degrees const other) const {return Degrees{data % other.data};}

  value_type data;
};

namespace literals {
constexpr Degrees operator"" _deg(unsigned long long value) {
  return Degrees{static_cast<Degrees::value_type>(value)};
}
}  // namespace user_defined_literals

}  // namespace numeric_types

#endif  // NUMERICTYPES_H
