// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "src/math/math.h"
#include "src/utils/string.h"

namespace warhol {

struct Vec2 {
  float x;
  float y;

  Vec2 operator+(const Vec2 rhs) const { return {x + rhs.x, y + rhs.y}; }
  Vec2 operator-(const Vec2 rhs) const { return {x - rhs.x, y - rhs.y}; }

  float* data() { return (float*)this; }
  std::string ToString() const { return StringPrintf("X: %f, Y: %f", x, y); }
  bool operator==(const Vec2& rhs) { return x == rhs.x && y == rhs.y; }
  bool operator!=(const Vec2& rhs) { return x != rhs.x || y != rhs.y; }
};

struct Vec3 {
  float x;
  float y;
  float z;

  float* data() { return (float*)this; }

  Vec3 operator+(const Vec3 rhs) const {
    return {x + rhs.x, y + rhs.y, z + rhs.z};
  }
  Vec3 operator-(const Vec3 rhs) const {
    return {x - rhs.x, y - rhs.y, z - rhs.z};
  }

  Vec3 operator-() const { return {-x, -y, -z}; }
  Vec3 operator*(float v) const { return {x * v, y * v, z * v}; }

  std::string ToString() const {
    return StringPrintf("X: %f, Y: %f, Z: %f", x, y, z);
  }

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


};




}  // namespace warhol
