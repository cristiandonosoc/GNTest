// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/utils/glm.h"

namespace warhol {

struct Camera {
  glm::mat4 projection;
  glm::mat4 view;
};

}  // namespace warhol
