// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/math/math.h"
#include "src/math/vec.h"

namespace warhol {

Vec3 DirectionFromEuler(float pitch, float yaw) {
  Vec3 direction;
  direction.x = std::cos(pitch) * std::cos(yaw);
  direction.y = std::sin(pitch);
  direction.z = std::cos(pitch) * sin(yaw);
  return direction.normalize();
}

Vec2 EulerFromDirection(const Vec3& direction) {
  // Project onto XZ plane.
  auto d_xz = Vec3{direction.x, 0, direction.z};
  float r_pitch = std::acos(direction.x / d_xz.mag());
  Vec2 result;
  result.x = radian2deg(r_pitch);   // Pitch.
  result.y = radian2deg(atan(direction.z / direction.x)); // Yaw.
  return result;
}

}  // namespace warhol
