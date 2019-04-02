// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/math/vec.h"
#include "warhol/utils/glm.h"

namespace warhol {

struct Camera {
  glm::mat4 projection;
  glm::mat4 view;

  Pair<int> viewport_p1;  // bottom-left.
  Pair<int> viewport_p2;  // top-right.
};

}  // namespace warhol
