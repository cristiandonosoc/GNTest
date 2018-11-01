// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "src/math/math.h"
#include "src/utils/string.h"

namespace warhol {

// Normally vectors are also used as pairs, but Vec2 and Vec3 are very much
// geared towards floats and didn't want to templatized them.
// Because std::pair has this annoying interface, I provide a pair alternative.
template <typename T>
struct Pair {
  T x;
  T y;

  T& min() { return x; }
  T& max() { return y; }

  Pair operator+(const Pair rhs) const { return {x + rhs.x, y + rhs.y}; }
  Pair operator-(const Pair rhs) const { return {x - rhs.x, y - rhs.y}; }

  std::string ToString() const { return StringPrintf("X: %f, Y: %f", (float)x, (float)y); }
  bool operator==(const Pair& rhs) const { return x == rhs.x && y == rhs.y; }
  bool operator!=(const Pair& rhs) const { return x != rhs.x || y != rhs.y; }

  bool operator<(const Pair& rhs) const { return x < rhs.x || y < rhs.y; }
  bool operator<=(const Pair& rhs) const { return *this < rhs || *this == rhs; }

  bool operator>(const Pair& rhs) const { return x > rhs.x || y > rhs.y; }
  bool operator>=(const Pair& rhs) const { return *this > rhs || *this == rhs; }
};

template <typename T>
struct HashPair {
  size_t operator()(const Pair<T> p) const {
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
};

// Mathematical vectors --------------------------------------------------------

using Vec2 = Pair<float>;

struct Vec3 {
  float x;
  float y;
  float z;

  float* data() { return (float*)this; }
  const float* data() const { return (const float*)this; }

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
  void operator*=(const Vec3 rhs) {
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
  }

  // Misc ----------------------------------------------------------------------

  std::string ToString() const {
    return StringPrintf("X: %f, Y: %f, Z: %f", x, y, z);
  }
};




}  // namespace warhol
