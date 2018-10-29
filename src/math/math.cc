// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/math/math.h"
#include "src/math/vec.h"

#include "src/utils/log.h"

namespace warhol {

namespace {

//          Z
//          ^
//      3   |   0
//          |
//   -------|-------> X
//          |
//      2   |   1
//
size_t XZQuadrant(float x, float z) {
  if (x >= 0.0f) {
    return z >= 0.0f ? 0 : 3;
  } else {
    return z >= 0.0f ? 1 : 2;
  }
}

}  // namespace

Vec3 DirectionFromEuler(float pitch, float yaw) {
  Vec3 direction;
  direction.x = std::cos(pitch) * std::cos(yaw);
  direction.y = std::sin(pitch);
  direction.z = std::cos(pitch) * sin(yaw);
  return direction.normalize();
}

Vec3 DirectionFromEulerDeg(float pitch, float yaw) {
  return DirectionFromEuler(deg2rad(pitch), deg2rad(yaw));
}

Pair<float> EulerFromDirection(const Vec3& direction) {
  Vec2 result;
  // Pitch.
  result.x = std::asin(direction.y);

  // Yaw.
  // Project onto XZ plane.
  auto d_xz = Vec3{direction.x, 0, direction.z};
  // This will gives the result in the first quadrant (0 to 90 deg in radians).
  // We need to offset the angle according to the actual quadrant.
  float angle = std::acos(direction.x / d_xz.mag());
  float offset = XZQuadrant(direction.x, direction.z) * Math::kPI / 2.0f;

  result.y = angle + offset;

  /* result.y = atan(direction.z / direction.x); */

  LOG(DEBUG) << "PITCH: " << result.x << ", YAW: " << result.y;

  return result;
}

Pair<float> EulerFromDirectionDeg(const Vec3& direction) {
  auto [pitch, yaw] = EulerFromDirection(direction);
  return {rad2deg(pitch), rad2deg(yaw)};
}

}  // namespace warhol
