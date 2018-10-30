// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/math/math.h"
#include "src/math/vec.h"

#include "src/utils/log.h"

namespace warhol {

Vec3 DirectionFromEulerDeg(float pitch, float yaw) {
  return DirectionFromEuler(deg2rad(pitch), deg2rad(yaw));
}

Vec3 DirectionFromEuler(float pitch, float yaw) {
  Vec3 direction;
  direction.x = std::cos(pitch) * std::cos(yaw);
  direction.y = std::sin(pitch);
  direction.z = std::cos(pitch) * std::sin(yaw);
  return direction.normalize();
}

Pair<float> EulerFromDirection(const Vec3& direction) {
  Vec2 result;
  // Pitch.
  result.x = std::asin(direction.y);

  // Yaw.
  // Thank god for atan2. I tried for a lot of time with a tan and some weird
  // acos of the dot product of xz. Now it works! :D
  result.y = std::atan2(direction.z, direction.x);
  return result;
}

Pair<float> EulerFromDirectionDeg(const Vec3& direction) {
  auto [pitch, yaw] = EulerFromDirection(direction);
  return {rad2deg(pitch), rad2deg(yaw)};
}

}  // namespace warhol
