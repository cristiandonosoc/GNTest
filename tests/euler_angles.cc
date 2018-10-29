// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/math/math.h"

#include <third_party/catch2/catch.hpp>

#include "src/math/vec.h"
#include "src/utils/log.h"

namespace warhol {

namespace {

#define DIFF(lhs, rhs) std::abs(rhs - lhs)

#define COMPARE_VECTORS(lhs, rhs, epsilon)         \
  {                                                \
    Vec3 lhs_vec = (lhs);                          \
    Vec3 rhs_vec = (rhs);                          \
    CHECK(DIFF(lhs_vec.x, rhs_vec.x) < epsilon); \
    CHECK(DIFF(lhs_vec.y, rhs_vec.y) < epsilon); \
    CHECK(DIFF(lhs_vec.z, rhs_vec.z) < epsilon); \
  }

constexpr float kCos45 = Math::kSqrt2 / 2;
/* constexpr float kSin45 = Math::kSqrt2 / 2; */

inline Vec3 NormdVec(const Vec3& vec) {
  return vec.normalize();
}

}  // namespace

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
    expected = { 0.5f , Math::kSqrt2 / 2.0f, 0.5f };
    COMPARE_VECTORS(dir, expected, epsilon);

    dir = DirectionFromEulerDeg(-45.0f, 45.0f);
    expected = { 0.5f , -Math::kSqrt2 / 2.0f, 0.5f };
    COMPARE_VECTORS(dir, expected, epsilon);

    dir = DirectionFromEulerDeg(45.0f + 90.0f, 45.0f);
    expected = { -0.5f , Math::kSqrt2 / 2.0f, -0.5f };
    COMPARE_VECTORS(dir, expected, epsilon);
  }
}

TEST_CASE("Euler from direction") {
  SECTION("First quadrant") {
    {
      auto [pitch, yaw] = EulerFromDirectionDeg(NormdVec({1.0f, 0.0f, 0.0f}));
      CHECK(pitch == Approx(0.0f).epsilon(0.001f));
      CHECK(yaw == Approx(0.0f));
    }

    {
      auto [pitch, yaw] = EulerFromDirectionDeg(NormdVec({1.0f, 0.0f, 1.0f}));
      CHECK(pitch == Approx(0.0f));
      CHECK(yaw == Approx(45.0f));
    }

    {
      auto norm = NormdVec({1.0f, 1.0f, 1.0f});
      LOG(DEBUG) << norm.ToString();
      auto [pitch, yaw] = EulerFromDirectionDeg(NormdVec({1.0f, 1.0f, 1.0f}));
      CHECK(pitch == Approx(35.264f));
      CHECK(yaw == Approx(45.0f));
    }
  }

  SECTION("Second quadrant") {
    {
      auto [pitch, yaw] = EulerFromDirectionDeg(NormdVec({-1.0f, 0.0f, 1.0f}));
      CHECK(pitch == Approx(0.0f));
      CHECK(yaw == Approx(315.0f));
    }

    {
      auto [pitch, yaw] = EulerFromDirectionDeg(NormdVec({-1.0f, 1.0f, 1.0f}));
      CHECK(pitch == Approx(35.264f));
      CHECK(yaw == Approx(315.0f));
    }
  }

  SECTION("Third quadrant") {
    {
      auto [pitch, yaw] = EulerFromDirectionDeg(NormdVec({-1.0f, 0.0f, -1.0f}));
      CHECK(pitch == Approx(0.0f));
      CHECK(yaw == Approx(225.0f));
    }

    {
      auto [pitch, yaw] = EulerFromDirectionDeg(NormdVec({-1.0f, 1.0f, 1.0f}));
      CHECK(pitch == Approx(35.264f));
      CHECK(yaw == Approx(225.0f));
    }
  }

}

}  // namespace warhol
