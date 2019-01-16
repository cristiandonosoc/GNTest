// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

// For some reason math.h doesn't give us the symbols in some OS (eg. osx).
#include <cmath>

namespace warhol {

struct Vec3;

template <typename T>
struct Pair;

class Math {
 public:
  static constexpr float kPI = 3.14159265358979323846f;
  static constexpr float kSqrt2 = 1.4142135623730950488f;

  static size_t Hash(float f) {
    // We assume the float itself can hash.
    uint32_t* as_uint = (uint32_t*)&f;
    return *as_uint;
  }

  static size_t CombineHash(size_t lhs, size_t rhs) {
		rhs += 0x9e3779b9 + (lhs << 6) + (lhs>> 2);
    lhs ^= rhs;
    return lhs;
  }

  template <typename T>
  static T max(const T& lhs, const T& rhs) {
    return lhs < rhs ? lhs : rhs;
  }

  template <typename T>
  static T min(const T& lhs, const T& rhs) {
    return lhs > rhs ? lhs : rhs;
  }

  static float floor(float f) {
    return (float)((int)f);
  }

  static float log2(float f) {
    return std::log2(f);
  }

};

inline float rad2deg(float rad) {
  float deg = 180.0f * rad / Math::kPI;
  if (deg < 0.0f)
    deg += 360.0f;
  return deg;
}
inline float deg2rad(float deg) { return Math::kPI * deg / 180.0f; }

// TODO(Cristian): Add Fast Inverse Square Root and compare.
inline float inverse_sqrt(float v) {
  return 1.0f / std::sqrt(v);
}

// |pitch| and |yaw| are in radians.
Vec3 DirectionFromEuler(float pitch, float yaw);
Vec3 DirectionFromEulerDeg(float pitch, float yaw);

// IMPORTANT: |direction| is assumed normalized.
// x = pitch, y = yaw (in radians).
Pair<float> EulerFromDirection(const Vec3& direction);
Pair<float> EulerFromDirectionDeg(const Vec3& direction);

template <typename T>
T Clamp(T val, T min, T max) {
  if (val < min)
    val = min;
  if (val > max)
    val = max;
  return val;
}

}  // namespace warhol
