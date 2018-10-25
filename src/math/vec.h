// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

namespace warhol {

template <typename T>
struct Vec2 {
  T x;
  T y;

  Vec2 operator+(const Vec2 rhs) const { return {x + rhs.x, y + rhs.y}; }
  Vec2 operator-(const Vec2 rhs) const { return {x - rhs.x, y - rhs.y}; }

  float* data() { return this; }
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

  float* data() { return (float*)this; }
};




}  // namespace warhol
