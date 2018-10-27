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
  Vec2 result;
  result.x = radian2deg(std::asin(direction.y));

  // Project onto XZ plane.
  auto d_xz = Vec3{direction.x, 0, direction.z};
  result.y = std::acos(direction.x / d_xz.mag());
  return result;
}

}  // namespace warhol
