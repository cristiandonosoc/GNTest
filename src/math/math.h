// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#ifdef _MSC_VER
#include <cmath>
#else
#include <math.h>
#endif

namespace warhol {

struct Vec3;

template <typename T> struct Pair;

class Math {
 public:
  static constexpr float kPI =  3.14159265358979323846f;
  static constexpr float kSqrt2 = 1.4142135623730950488f;
};

inline float rad2deg(float rad) { return 180.0f * rad / Math::kPI; }
inline float deg2rad(float deg) { return Math::kPI * deg / 180.0f; }

// TODO(Cristian): Add Fast Inverse Square Root and compare.
inline float inverse_sqrt(float v) {
  return 1.0f / std::sqrt(v);
}

// |pitch| and |yaw| are in radians.
Vec3 DirectionFromEuler(float pitch, float yaw);
Vec3 DirectionFromEulerDeg(float pitch, float yaw);

// x = pitch, y = yaw (in radians).
Pair<float> EulerFromDirection(const Vec3& direction);

}  // namespace warhol
