// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "src/utils/string.h"

namespace warhol {

template <typename T>
struct Vec2 {
  T x;
  T y;

  Vec2 operator+(const Vec2 rhs) const { return {x + rhs.x, y + rhs.y}; }
  Vec2 operator-(const Vec2 rhs) const { return {x - rhs.x, y - rhs.y}; }

  float* data() { return this; }
  std::string ToString() const { return StringPrintf("X: %f, Y: %f", x, y); }
  bool operator==(const Vec2<T>& rhs) { return x == rhs.x && y == rhs.y; }
  bool operator!=(const Vec2<T>& rhs) { return x != rhs.x || y != rhs.y; }
};

template <typename T>
struct Vec3 {
  T x;
  T y;
  T z;

  Vec3 operator+(const Vec3 rhs) const {
    return {x + rhs.x, y + rhs.y, z + rhs.z};
  }
  Vec3 operator-(const Vec3 rhs) const {
    return {x - rhs.x, y - rhs.y, z - rhs.z};
  }

  Vec3 operator-() const { return {-x, -y, -z}; }

  float* data() { return (float*)this; }
  std::string ToString() const {
    return StringPrintf("X: %f, Y: %f, Z: %f", x, y, z);
  }
};




}  // namespace warhol
