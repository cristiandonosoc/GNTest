// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <assert.h>

#include "warhol/math/math.h"
#include "warhol/utils/string.h"

namespace warhol {

// New Implementation ----------------------------------------------------------

template <typename T>
struct _v2 {
  T x = 0;
  T y = 0;

  static _v2 Zero() { return {}; }

  _v2 operator+(const _v2& o) const { return {x + o.x, y + o.y}; }
  void operator+=(const _v2& o) { x += o.x; y += o.y; }

  bool operator==(const _v2& o) const { return x == o.x && y == o.y; }
  bool operator!=(const _v2& o) const { return x != o.x || y != o.y; }
};

template <typename T>
inline bool IsZero(const _v2<T>& v) { return v.x == 0 && v.y == 0; }

using Int2 = _v2<int>;

template <typename T>
inline std::string ToString(const _v2<T>& v) {
  return StringPrintf("X: %f, Y: %f", (float)v.x, (float)v.y);
}

template <typename T>
struct _v3 {
  T x, y, z;
};

using Int3 = _v3<int>;

template<typename T>
struct _v4 {
  T x, y, z, w;
};

template <typename T>
inline float Sum(const _v4<T>& v) {
  return v.x + v.y + v.z + v.w;
}

using Vec4 = _v4<float>;
using Int4 = _v4<int>;

// Old implementation ----------------------------------------------------------


// Normally vectors are also used as pairs, but Vec2 and Vec3 are very much
// geared towards floats and didn't want to templatized them.
// Because std::pair has this annoying interface, I provide a pair alternative.
template <typename T>
struct Pair {
  T x;
  T y;

  T& min() { return x; }
  T& max() { return y; }

  T& operator[](size_t index) {
    assert(index < 3);
    return *((T*)this + index);
  }

  Pair operator+(const Pair& rhs) const { return {x + rhs.x, y + rhs.y}; }
  Pair operator-(const Pair& rhs) const { return {x - rhs.x, y - rhs.y}; }

  bool operator==(const Pair& rhs) const { return x == rhs.x && y == rhs.y; }
  bool operator!=(const Pair& rhs) const { return x != rhs.x || y != rhs.y; }

  bool operator<(const Pair& rhs) const { return x < rhs.x || y < rhs.y; }
  bool operator<=(const Pair& rhs) const { return *this < rhs || *this == rhs; }

  bool operator>(const Pair& rhs) const { return x > rhs.x || y > rhs.y; }
  bool operator>=(const Pair& rhs) const { return *this > rhs || *this == rhs; }

  std::string ToString() const {
    return StringPrintf("X: [%s], Y: [%s]",
                        x.ToString().data(), y.ToString().data());
  }

};


template <>
std::string Pair<int>::ToString() const;

template <>
std::string Pair<uint32_t>::ToString() const;

template <>
std::string Pair<float>::ToString() const;


template <typename T>
struct HashPair {
  size_t operator()(const Pair<T>& p) const {
    // TODO(Cristian): Do some more decent hash function.
    size_t result = 2166136261;
    result = (((result * 16777619) ^ p.x) ^ p.y) << 1;
    return result;
  }
};

template <typename T>
struct Pair3 {
  T x;
  T y;
  T z;

  T& operator[](size_t index) {
    assert(index < 3);
    return *((T*)this + index);
  }

  Pair3 operator+(const Pair3& rhs) const {
    return {x + rhs.x, y + rhs.y, z + rhs.z};
  }
  Pair3 operator-(const Pair3& rhs) const {
    return {x - rhs.x, y - rhs.y, z - rhs.z};
  }

  bool operator==(const Pair3& rhs) const {
    return x == rhs.x && y == rhs.y && z == rhs.z;
  }
  bool operator!=(const Pair3& rhs) const {
    return x != rhs.x || y != rhs.y || z != rhs.z;
  }

  bool operator<(const Pair3& rhs) const {
    return x < rhs.x || y < rhs.y || z < rhs.z;
  }
  bool operator<=(const Pair3& rhs) const {
    return *this < rhs || *this == rhs;
  }

  bool operator>(const Pair3& rhs) const {
    return x > rhs.x || y > rhs.y || z > rhs.z;
  }
  bool operator>=(const Pair3& rhs) const {
    return *this > rhs || *this == rhs;
  }

  void operator*=(T v) {
    x *= v;
    y *= v;
    z *= v;
  }

  std::string ToString() const {
    return StringPrintf("X: %f, Y: %f, Z: %f", (float)x, (float)y, (float)z);
  }

};

template<typename T>
struct Quad3 {
  Pair3<T> min;
  Pair3<T> max;

  std::string ToString() const {
    return StringPrintf("MIN: [%s], MAX: [%s]", min.ToString().data(),
                                                max.ToString().data());
  }
};

template <typename T>
struct HashPair3 {
  size_t operator()(const Pair3<T>& p) const {
    // TODO(Cristian): Do some more decent hash function.
    size_t result = 2166136261;
    result = ((((result * 16777619) ^ p.x) ^ p.y) << 1) ^ p.z << 1;
    return result;
  }
};

// Mathematical vectors --------------------------------------------------------

using Vec2 = Pair<float>;

struct Vec3 {
  float x = 0;
  float y = 0;
  float z = 0;

  Vec3() = default;
  Vec3(int x, int y, int z) : x((float)x), y((float)y), z((float)z) {}
  Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

  float* data() { return (float*)this; }
  const float* data() const { return (const float*)this; }

  // Operators -----------------------------------------------------------------

  float& operator[](size_t i) {
    assert(i < 3);
    float* as_float = reinterpret_cast<float*>(this);
    return *(as_float + i);
  }

  // Vector Operations ---------------------------------------------------------

  float dot(const Vec3& rhs) const {
    return x * rhs.x + y * rhs.y + z * rhs.z;
  }

  float mag() const {
    return std::sqrt(x*x + y*y + z*z);
  }

  // Magnitude squared.
  float mag2() const {
    return dot(*this);
  }

  Vec3 normalize() const {
    // TODO(Cristian): Consider a variant that uses fast_inverse_sqrt.
    return *this * inverse_sqrt(dot(*this));
  }

  Vec3 cross(const Vec3& rhs) const {
    return {this->y * rhs.z - rhs.y * this->z,
            rhs.x * this->z - this->x * rhs.z,
            this->x * rhs.y - rhs.x * this->y};
  }

  // Operators -----------------------------------------------------------------

  bool operator==(const Vec3& rhs) const {
    return x == rhs.x && y == rhs.y && z == rhs.z;
  }

  bool operator!=(const Vec3& rhs) const {
    return x != rhs.x || y != rhs.y || z != rhs.z;
  }

  bool operator<(const Vec3& rhs) const {
    if (x != rhs.x) return x < rhs.x;
    if (y != rhs.y) return y < rhs.y;
    return z < rhs.z;
  }

  Vec3 operator-() const { return {-x, -y, -z}; }

  Vec3 operator+(const Vec3 rhs) const {
    return {x + rhs.x, y + rhs.y, z + rhs.z};
  }
  void operator+=(const Vec3 rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
  }

  Vec3 operator-(const Vec3 rhs) const {
    return {x - rhs.x, y - rhs.y, z - rhs.z};
  }
  void operator-=(const Vec3 rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
  }

  Vec3 operator*(float v) const { return {x * v, y * v, z * v}; }
  void operator*=(float v) {
    x *= v;
    y *= v;
    z *= v;
  }

  // Misc ----------------------------------------------------------------------

  std::string ToString() const {
    return StringPrintf("X: %f, Y: %f, Z: %f", x, y, z);
  }
};

inline size_t Hash(const Vec2& vec) {
  size_t result = 0;
  result = Math::CombineHash(result, Math::Hash(vec.x));
  result = Math::CombineHash(result, Math::Hash(vec.y));
  return result;
}

inline size_t Hash(const Vec3& vec) {
  size_t result = 0;
  result = Math::CombineHash(result, Math::Hash(vec.x));
  result = Math::CombineHash(result, Math::Hash(vec.y));
  result = Math::CombineHash(result, Math::Hash(vec.z));
  return result;
}

}  // namespace warhol
