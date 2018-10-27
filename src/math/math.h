// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <math.h>

namespace warhol {

struct Vec2;
struct Vec3;

class Math {
 public:
  static constexpr float kPI =  3.14159265358979323846f;
};

inline float radian2deg(float radian) {
  return 180.0f * radian / Math::kPI;
}


// TODO(Cristian): Add Fast Inverse Square Root and compare.
inline float inverse_sqrt(float v) {
  return 1.0f / std::sqrt(v);
}

// |pitch| and |yaw| are in radians.
Vec3 DirectionFromEuler(float pitch, float yaw);

// x = pitch, y = yaw
Vec2 EulerFromDirection(const Vec3& direction);

}  // namespace warhol
