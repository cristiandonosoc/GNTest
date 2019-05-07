// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <assert.h>

#include <array>

#include "warhol/math/math.h"
#include "warhol/utils/log.h"
#include "warhol/utils/string.h"

namespace warhol {

// New Implementation ----------------------------------------------------------

// Vec 2 -----------------------------------------------------------------------

template <typename T>
union _v2 {
  struct { T x, y; };
  struct { T u, v; };
  struct { T left, right; };
  struct { T min, max; };
  struct { T width, height; };
  T elements[2];

  // Operators

  _v2() = default;
  _v2(T x, T y) { this->x = x; this->y = y; }

  static _v2 Zero() { return {0, 0}; }

  T& operator[](int index) {
    ASSERT(index >= 0 && index < 2);
    return elements[index];
  }
  const T& operator[](int index) const {
    return (*((_v2*)(this)))[index];
  }

  _v2 operator+(const _v2& o) const { return {x + o.x, y + o.y}; }
  void operator+=(const _v2& o) { x += o.x; y += o.y; }
  _v2 operator-(const _v2& o) const { return {x - o.x, y - o.y}; }
  void operator-=(const _v2& o) { x -= o.x; y -= o.y; }

  // * is dot product.
  T operator*(const _v2& o) const { return x * o.x + y * o.y; }

  bool operator==(const _v2& o) const { return x == o.x && y == o.y; }
  bool operator!=(const _v2& o) const { return x != o.x || y != o.y; }
};

using Int2 = _v2<int>;

template <typename T>
inline bool IsZero(const _v2<T>& v) { return v.x == 0 && v.y == 0; }

template <typename T>
inline std::string ToString(const _v2<T>& v) {
  return StringPrintf("(%f, %f)", (float)v.x, (float)v.y);
}

// Vec 3 -----------------------------------------------------------------------

template <typename T>
union _v3 {
  struct { T x, y, z; };
  struct { T u, v, w; };
  struct { T r, g, b; };
  T elements[3];

  // Operators

  _v3() = default;
  _v3(T x, T y, T z) { this->x = x; this->y = y; this->z = z; }

  static _v3 Zero() { return {0, 0, 0}; }

  _v3 operator+(const _v3& o) const { return {x + o.x, y + o.y, z + o.z}; }
  void operator+=(const _v3& o) { x += o.x; y += o.y; z += o.z; }

  void operator==(const _v3& o) const {
    return x == o.x && y == o.y && z == o.z;
  }
  void operator!=(const _v3& o) const {
    return x != o.x || y != o.y || z != o.z;
  }
};

using Int3 = _v3<int>;

template <typename T>
inline bool IsZero(const _v3<T>& v) { return v.x == 0 && v.y == 0 && v.z == 0; }

template <typename T>
inline std::string ToString(const _v3<T>& v) {
  return StringPrintf("(%f, %f, %f)", (float)v.x, (float)v.y, (float)v.z);
}

// Vec 4 -----------------------------------------------------------------------

template<typename T>
struct _v4 {
  struct { T x, y, z, w; };
  struct { T r, g, b, a; };
  T elements[4];

  // Operators

  _v4() = default;
  _v4(T _x, T _y, T _z, T _w) { x = _x; y = _y; z = _z; w = _w; }

  _v4 operator+(const _v4& o) const {
    return {x + o.x, y + o.y, z + o.z, w + o.w};
  }
};

using Vec4 = _v4<float>;
using Int4 = _v4<int>;

template <typename T>
inline std::string ToString(const _v4<T>& v) {
  return StringPrintf("(%f, %f, %f, %f)",
                      (float)v.x, (float)v.y, (float)v.z, (float)v.w);
}

template <typename T>
inline float Sum(const _v4<T>& v) {
  return v.x + v.y + v.z + v.w;
}

// Matrices --------------------------------------------------------------------
//
// Matrices are implemented as column mayor.

template <typename T>
union _mat2 {
  _v2<T> columns[2];

  // Operators -----------------------------------------------------------------

  _mat2() = default;
  _mat2(_v2<T> c1, _v2<T> c2) { columns[0] = c1; columns[1] = c2; }

  static _mat2 FromRows(_v2<T> r1, _v2<T> r2) {
    return _mat2({r1[0], r2[0]}, {r1[1], r2[1]});
  }

  _v2<T>& operator[](int index) {
    ASSERT(index >= 0 && index < 2);
    return columns[index];
  }

  _v2<T> operator*(const _v2<T>& v) const {
    return {columns[0][0] * v[0] + columns[1][0] * v[1],
            columns[0][1] * v[0] + columns[1][1] * v[1]};
  }
};

template <typename T>
std::array<T, 4> ToRowArray(_mat2<T>* mat) {
  std::array<T, 4> array;
  array[0] = (*mat)[0][0];
  array[1] = (*mat)[1][0];
  array[2] = (*mat)[0][1];
  array[3] = (*mat)[1][1];
  return array;
}

using IntMat2 = _mat2<int>;
using Mat2 = _mat2<float>;

// Boxes -----------------------------------------------------------------------

template <typename T>
struct _box {
  T min;
  T max;
};

using IntBox2 = _box<Int2>;

template <typename T>
inline std::string ToString(const _box<T>& box) {
  return StringPrintf("MIN: (%f, %f), MAX: (%f, %f)",
                      (float)box.min.x, (float)box.min.y,
                      (float)box.max.x, (float)box.max.y);
}

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
