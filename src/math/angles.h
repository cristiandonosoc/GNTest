// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <math.h>

namespace warhol {

inline float radian2deg(float radian) {
  return 180.0f * radian / M_PI;
}

}  // namespace warhol
