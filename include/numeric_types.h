#ifndef NUMERIC_TYPES_H
#define NUMERIC_TYPES_H

#include <nds/ndstypes.h>

namespace numeric_types {

// Represents a fixed point integral type.
// T should be an integral type; it is exposed because fixeds may be any number
// of bits. F is the number of fractional bits.
// Because this is a numeric type, all functions are defined inline in hopes
// that the compiler will inline the short underlying functions.
template <typename T, int F>
class Fixed {
  template<typename T2, int F2>
  friend class Fixed;

 public:
  Fixed() {
    data = 0;
  }
  ~Fixed() {}

  // Initialization from other types
  Fixed(const int& other) {*this = other;}
  Fixed(const float& other) {*this = other;}
  template <typename T2, int F2>
  Fixed(const Fixed<T2, F2>& other) {*this = other;}

  Fixed<T, F>& operator=(const int& other) {data = other << F; return *this;}
  Fixed<T, F>& operator=(const float& other) {data = (int)(other * (1 << F)); return *this;}
  template <typename T2, int F2>
  Fixed<T, F>& operator=(const Fixed<T2, F2>& other) {
    data = F > F2 ?
        other.data << AbsoluteDifference<F, F2>::value :
        other.data >> AbsoluteDifference<F, F2>::value;
    return *this;
  }

  // Comparison
  bool operator==(const Fixed<T, F>& other) const {return data == other.data;}
  bool operator<(const Fixed<T, F>& other) const {return data < other.data;}
  bool operator>(const Fixed<T, F>& other) const {return data > other.data;}
  bool operator<=(const Fixed<T, F>& other) const {return data <= other.data;}
  bool operator>=(const Fixed<T, F>& other) const {return data >= other.data;}
  bool operator!=(const Fixed<T, F>& other) const {return !(data == other.data);}

  // Comparison with ints
  bool operator==(const int& other) const {return data == other << F;}
  bool operator<(const int& other) const {return data < other << F;}
  bool operator>(const int& other) const {return data > other << F;}
  bool operator<=(const int& other) const {return data <= other << F;}
  bool operator>=(const int& other) const {return data >= other << F;}
  bool operator!=(const int& other) const {return !(data == other << F);}

  // Comparison with floats
  bool operator==(const float& other) const {return data == ((Fixed<T, F>)other).data;}
  bool operator<(const float& other) const {return data < ((Fixed<T, F>)other).data;}
  bool operator>(const float& other) const {return data > ((Fixed<T, F>)other).data;}
  bool operator<=(const float& other) const {return data <= ((Fixed<T, F>)other).data;}
  bool operator>=(const float& other) const {return data >= ((Fixed<T, F>)other).data;}
  bool operator!=(const float& other) const {return !(data == ((Fixed<T, F>)other).data);}

  // Addition and subtraction
  Fixed<T, F> operator+(const Fixed<T, F>& other) {Fixed<T,F> r; r.data = data + other.data; return r;}
  Fixed<T, F>& operator+=(const Fixed<T, F>& other) {data += other.data; return *this;}
  Fixed<T, F> operator-(const Fixed<T, F>& other) {Fixed<T,F> r; r.data = data - other.data; return r;}
  Fixed<T, F>& operator-=(const Fixed<T, F>& other) {data -= other.data; return *this;}

  // Multiplication and division
  Fixed<T, F> operator*(const Fixed<T, F>& other) {Fixed<T,F> r; r.data = ((s64)data * (s64)other.data) >> F; return r;}
  Fixed<T, F>& operator*=(const Fixed<T, F>& other) {data = ((s64)data * (s64)other.data) >> F; return *this;}
  Fixed<T, F> operator/(const Fixed<T, F>& other) {Fixed<T,F> r; r.data = (data << F) / (other.data); return r;}
  Fixed<T, F>& operator/=(const Fixed<T, F>& other) {data = (data << F) / (other.data); return *this;}

  // Type conversion
  explicit operator int() const {return data >> F;}
  explicit operator float() const {return ((float)data) / (float)(1 << F);}

  T data;

 private:
  // Calculates absolute difference of two compile time constants.
  // Written to silence warnings from the compiler about negative shifts that
  // could occur (but would never be executed) in code when working with Fixed
  // values with differing fractional parts.
  template<int V, int V2>
  struct AbsoluteDifference {
    enum { value = AbsoluteDifference<V - 1, V2 - 1>::value };
  };

  template<int V>
  struct AbsoluteDifference<V, 0> {
    enum { value = V };
  };

  template<int V2>
  struct AbsoluteDifference<0, V2> {
    enum { value = V2 };
  };
};

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

#endif  // NUMERIC_TYPES_H
