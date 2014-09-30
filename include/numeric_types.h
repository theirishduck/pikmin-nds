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

  constexpr Degrees() : data{0} {}
  constexpr Degrees(Degrees const& other) : data{other.data} {}
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

  constexpr Degrees Div(Degrees const other) const {return Degrees{data / other.data};}
  constexpr Degrees Mod(Degrees const other) const {return Degrees{data % other.data};}

  value_type data;
};

namespace literals {
constexpr Degrees operator"" _deg(unsigned long long value) {
  return Degrees{static_cast<Degrees::value_type>(value)};
}
}  // namespace literals

class Brads
{
public:
  using value_type = s16;

  constexpr Brads() : data{0} {}
  constexpr Brads(Brads const& other) : data{other.data} {}
  static constexpr Brads Raw(value_type const value) {return Brads{value};}
  Brads& operator=(Brads const other) {data = other.data; return *this;}

  constexpr bool operator==(Brads const other) const {return data == other.data;}
  constexpr bool operator<(Brads const other) const {return data < other.data;}
  constexpr bool operator>(Brads const other) const {return data > other.data;}
  constexpr bool operator<=(Brads const other) const {return data <= other.data;}
  constexpr bool operator>=(Brads const other) const {return data >= other.data;}
  constexpr bool operator!=(Brads const other) const {return data != other.data;}

  constexpr Brads operator+(Brads const other) const {return Brads{static_cast<value_type>(data + other.data)};}
  Brads& operator+=(Brads const other) {data += other.data; return *this;}
  constexpr Brads operator-(Brads const other) const {return Brads{static_cast<value_type>(data - other.data)};}
  Brads& operator-=(Brads const other) {data -= other.data; return *this;}
  constexpr Brads operator*(Brads const other) const {return Brads{static_cast<value_type>(data * other.data)};}
  Brads& operator*=(Brads const other) {data *= other.data; return *this;}
  Brads const operator/(Brads const other) const {return Brads{static_cast<value_type>(div32(data, other.data))};}
  Brads& operator/=(Brads const other) {data = static_cast<value_type>(div32(data, other.data)); return *this;}
  Brads const operator%(Brads const other) const {return Brads{static_cast<value_type>(mod32(data, other.data))};}
  Brads& operator%=(Brads const other) {data = static_cast<value_type>(mod32(data, other.data)); return *this;}

  constexpr Brads Div(Brads const other) const {return Brads{static_cast<value_type>(data / other.data)};}
  constexpr Brads Mod(Brads const other) const {return Brads{static_cast<value_type>(data % other.data)};}

  value_type data;

private:
  explicit constexpr Brads(value_type const value) : data{value} {}
};

namespace literals {
constexpr Brads operator"" _brad(long double value) {
  return Brads::Raw(static_cast<Brads::value_type>(degreesToAngle(value)));
}

constexpr Brads operator"" _brad(unsigned long long value) {
  return Brads::Raw(static_cast<Brads::value_type>(degreesToAngle(value)));
}
}  // namespace literals

}  // namespace numeric_types

#endif  // NUMERICTYPES_H
