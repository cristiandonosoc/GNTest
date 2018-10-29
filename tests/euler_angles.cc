// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/math/math.h"
#include "src/math/vec.h"

#include <third_party/catch2/catch.hpp>

namespace warhol {

#define DIFF(lhs, rhs) std::abs(rhs - lhs)

#define COMPARE_VECTORS(lhs, rhs, epsilon)         \
  {                                                \
    Vec3 lhs_vec = (lhs);                          \
    Vec3 rhs_vec = (rhs);                          \
    REQUIRE(DIFF(lhs_vec.x, rhs_vec.x) < epsilon); \
    REQUIRE(DIFF(lhs_vec.y, rhs_vec.y) < epsilon); \
    REQUIRE(DIFF(lhs_vec.z, rhs_vec.z) < epsilon); \
  }

static constexpr float kCos45 = Math::kSqrt2 / 2;
static constexpr float kSin45 = Math::kSqrt2 / 2;

TEST_CASE("Direction From Euler") {
  Vec3 dir;
  Vec3 expected;
  float epsilon = 0.05f;

  SECTION("Only pitch") {
    dir = DirectionFromEulerDeg(0.0f, 0.0f);
    expected = {1.0f, 0.0f, 0.0f};
    COMPARE_VECTORS(dir, expected, epsilon);

    dir = DirectionFromEulerDeg(45.0f, 0.0f);
    expected = {kCos45, kCos45, 0.0f};
    COMPARE_VECTORS(dir, expected, epsilon);

    dir = DirectionFromEulerDeg(180.0f, 0.0f);
    expected =  {-1.0f, 0.0f, 0.0f};
    COMPARE_VECTORS(dir, expected, epsilon);
  }

  SECTION("Only yaw") {
    dir = DirectionFromEulerDeg(0.0f, 45.0f);
    expected = { kCos45, 0.0f, kCos45 };
    COMPARE_VECTORS(dir, expected, epsilon);

    dir = DirectionFromEulerDeg(0.0f, 180.0f);
    expected =  {-1.0f, 0.0f, 0.0f};
    COMPARE_VECTORS(dir, expected, epsilon);
  }

  SECTION("Pitch & Yaw") {
    dir = DirectionFromEulerDeg(45.0f, 45.0f);
    expected = { kCos45 * kCos45, kSin45, kCos45 * kSin45 };
    COMPARE_VECTORS(dir, expected, epsilon);

    dir = DirectionFromEulerDeg(-45.0f, 45.0f);
    expected = { kCos45 * kCos45, -kSin45, kCos45 * kSin45 };
    COMPARE_VECTORS(dir, expected, epsilon);

    dir = DirectionFromEulerDeg(45.0f + 90.0f, 45.0f);
    expected = { -kCos45 * kCos45, kSin45, -kCos45 * kSin45 };
    COMPARE_VECTORS(dir, expected, epsilon);
  }
}


}  // namespace warhol
