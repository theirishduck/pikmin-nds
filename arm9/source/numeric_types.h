#ifndef NUMERIC_TYPES_H
#define NUMERIC_TYPES_H

#include <nds/arm9/math.h>
#include <nds/arm9/trig_lut.h>
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
 private:
  constexpr Fixed(const int& other) : data_(other) {}

 public:
  Fixed() {
    data_ = 0;
  }

  static constexpr Fixed<T, F> FromInt(const int& value) {return Fixed<T,F>(value << F);}
  static constexpr Fixed<T, F> FromFloat(const float& value) {return Fixed<T,F>(value * (1 << F));}

  // Initialization from other types
  template <typename T2, int F2>
  Fixed(const Fixed<T2, F2>& other) {*this = other;}

  //Fixed<T, F>& operator=(const int& other) {data_ = other << F; return *this;}
  //Fixed<T, F>& operator=(const float& other) {data_ = (int)(other * (1 << F)); return *this;}
  template <typename T2, int F2>
  Fixed<T, F>& operator=(const Fixed<T2, F2>& other) {
    data_ = F > F2 ?
        other.data_ << AbsoluteDifference<F, F2>::value :
        other.data_ >> AbsoluteDifference<F, F2>::value;
    return *this;
  }

  // Comparison
  bool operator==(const Fixed<T, F>& other) const {return data_ == other.data_;}
  bool operator<(const Fixed<T, F>& other) const {return data_ < other.data_;}
  bool operator>(const Fixed<T, F>& other) const {return data_ > other.data_;}
  bool operator<=(const Fixed<T, F>& other) const {return data_ <= other.data_;}
  bool operator>=(const Fixed<T, F>& other) const {return data_ >= other.data_;}
  bool operator!=(const Fixed<T, F>& other) const {return !(data_ == other.data_);}

  // Comparison with ints
  //bool operator==(const int& other) const {return data_ == other << F;}
  //bool operator<(const int& other) const {return data_ < other << F;}
  //bool operator>(const int& other) const {return data_ > other << F;}
  //bool operator<=(const int& other) const {return data_ <= other << F;}
  //bool operator>=(const int& other) const {return data_ >= other << F;}
  //bool operator!=(const int& other) const {return !(data_ == other << F);}

  // Comparison with floats
  //bool operator==(const float& other) const {return data_ == ((Fixed<T, F>)other).data_;}
  //bool operator<(const float& other) const {return data_ < ((Fixed<T, F>)other).data_;}
  //bool operator>(const float& other) const {return data_ > ((Fixed<T, F>)other).data_;}
  //bool operator<=(const float& other) const {return data_ <= ((Fixed<T, F>)other).data_;}
  //bool operator>=(const float& other) const {return data_ >= ((Fixed<T, F>)other).data_;}
  //bool operator!=(const float& other) const {return !(data_ == ((Fixed<T, F>)other).data_);}

  // Addition and subtraction
  Fixed<T, F> operator+(const Fixed<T, F>& other) {Fixed<T,F> r; r.data_ = data_ + other.data_; return r;}
  Fixed<T, F>& operator+=(const Fixed<T, F>& other) {data_ += other.data_; return *this;}
  Fixed<T, F> operator-(const Fixed<T, F>& other) {Fixed<T,F> r; r.data_ = data_ - other.data_; return r;}
  Fixed<T, F>& operator-=(const Fixed<T, F>& other) {data_ -= other.data_; return *this;}

  // Multiplication and division
  Fixed<T, F> operator*(const Fixed<T, F>& other) {Fixed<T,F> r; r.data_ = ((s64)data_ * (s64)other.data_) >> F; return r;}
  Fixed<T, F>& operator*=(const Fixed<T, F>& other) {data_ = ((s64)data_ * (s64)other.data_) >> F; return *this;}
  Fixed<T, F> operator/(const Fixed<T, F>& other) {Fixed<T,F> r; r.data_ = (data_ << F) / (other.data_); return r;}
  Fixed<T, F>& operator/=(const Fixed<T, F>& other) {data_ = (data_ << F) / (other.data_); return *this;}

  //unary negation
  constexpr Fixed operator-() { return Fixed{-data_}; }

  // Type conversion
  explicit operator int() const {return data_ >> F;}
  explicit operator float() const {return ((float)data_) / (float)(1 << F);}

  T data_;

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

// Because this form of fixed is so common
using fixed = numeric_types::Fixed<s32, 12>;

namespace literals {
constexpr fixed operator"" _f(unsigned long long value) {
  return fixed::FromInt(static_cast<s32>(value));
}

constexpr fixed operator"" _f(long double value) {
  return fixed::FromFloat(static_cast<float>(value));
}
}  // namespace literals

// Hardware division overloads
/*
template<>
fixed Fixed<s32, 12>::operator/(const fixed& other);// {fixed r; r.data_ = divf32(data_, other.data_); return r;}
template<>
fixed& Fixed<s32, 12>::operator/=(const fixed& other);// {data_ = divf32(data_, other.data_); return *this;}
*/
class Degrees
{
 public:
  using value_type = s32;

  constexpr Degrees() : data_{0} {}
  constexpr Degrees(Degrees const& other) : data_{other.data_} {}
  explicit constexpr Degrees(value_type const value) : data_{value} {}
  Degrees& operator=(Degrees const other) {data_ = other.data_; return *this;}

  constexpr bool operator==(Degrees const other) const {return data_ == other.data_;}
  constexpr bool operator<(Degrees const other) const {return data_ < other.data_;}
  constexpr bool operator>(Degrees const other) const {return data_ > other.data_;}
  constexpr bool operator<=(Degrees const other) const {return data_ <= other.data_;}
  constexpr bool operator>=(Degrees const other) const {return data_ >= other.data_;}
  constexpr bool operator!=(Degrees const other) const {return data_ != other.data_;}

  constexpr Degrees operator+(Degrees const other) const {return Degrees{data_ + other.data_};}
  Degrees& operator+=(Degrees const other) {data_ += other.data_; return *this;}
  constexpr Degrees operator-(Degrees const other) const {return Degrees{data_ - other.data_};}
  Degrees& operator-=(Degrees const other) {data_ -= other.data_; return *this;}
  constexpr Degrees operator*(Degrees const other) const {return Degrees{data_ * other.data_};}
  Degrees& operator*=(Degrees const other) {data_ *= other.data_; return *this;}
  Degrees const operator/(Degrees const other) const {return Degrees{div32(data_, other.data_)};}
  Degrees& operator/=(Degrees const other) {data_ = div32(data_, other.data_); return *this;}
  Degrees const operator%(Degrees const other) const {return Degrees{mod32(data_, other.data_)};}
  Degrees& operator%=(Degrees const other) {data_ = mod32(data_, other.data_); return *this;}

  constexpr Degrees Div(Degrees const other) const {return Degrees{data_ / other.data_};}
  constexpr Degrees Mod(Degrees const other) const {return Degrees{data_ % other.data_};}

  value_type data_;
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

  constexpr Brads() : data_{0} {}
  constexpr Brads(Brads const& other) : data_{other.data_} {}
  static constexpr Brads Raw(value_type const value) {return Brads{value};}
  Brads& operator=(Brads const other) {data_ = other.data_; return *this;}

  constexpr bool operator==(Brads const other) const {return data_ == other.data_;}
  constexpr bool operator<(Brads const other) const {return data_ < other.data_;}
  constexpr bool operator>(Brads const other) const {return data_ > other.data_;}
  constexpr bool operator<=(Brads const other) const {return data_ <= other.data_;}
  constexpr bool operator>=(Brads const other) const {return data_ >= other.data_;}
  constexpr bool operator!=(Brads const other) const {return data_ != other.data_;}

  constexpr Brads operator+(Brads const other) const {return Brads{static_cast<value_type>(data_ + other.data_)};}
  Brads& operator+=(Brads const other) {data_ += other.data_; return *this;}
  constexpr Brads operator-(Brads const other) const {return Brads{static_cast<value_type>(data_ - other.data_)};}
  Brads& operator-=(Brads const other) {data_ -= other.data_; return *this;}
  constexpr Brads operator*(Brads const other) const {return Brads{static_cast<value_type>(data_ * other.data_)};}
  Brads& operator*=(Brads const other) {data_ *= other.data_; return *this;}
  Brads const operator/(Brads const other) const {return Brads{static_cast<value_type>(div32(data_, other.data_))};}
  Brads& operator/=(Brads const other) {data_ = static_cast<value_type>(div32(data_, other.data_)); return *this;}
  Brads const operator%(Brads const other) const {return Brads{static_cast<value_type>(mod32(data_, other.data_))};}
  Brads& operator%=(Brads const other) {data_ = static_cast<value_type>(mod32(data_, other.data_)); return *this;}

  //unary negation
  constexpr Brads operator-() { return Brads{static_cast<value_type>(-data_)}; }

  constexpr Brads Div(Brads const other) const {return Brads{static_cast<value_type>(data_ / other.data_)};}
  constexpr Brads Mod(Brads const other) const {return Brads{static_cast<value_type>(data_ % other.data_)};}

  value_type data_;

 private:
  explicit constexpr Brads(value_type const value) : data_{value} {}
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
