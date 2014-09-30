#ifndef FIXED_H
#define FIXED_H

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

}  // namespace gx

#endif  // FIXED_H
