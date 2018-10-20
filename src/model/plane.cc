// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/model/plane.h"

#include <math.h>
#include <stdlib.h>

namespace warhol {

std::vector<float> Plane::Create(float width, float length) {
  float fwidth = (float)floor(width) / 2;
  float flength = (float)floor(length) / 2;

  // UV is same as vertex position, so we can use the repeat pattern.
  auto vertices = std::vector<float>{
    // Position              // UV
     fwidth, 0.0f, -flength,  fwidth, -flength,
     fwidth, 0.0f,  flength,  fwidth,  flength,
    -fwidth, 0.0f, -flength, -fwidth, -flength,
    -fwidth, 0.0f,  flength, -fwidth,  flength,
  };


  return vertices;
}

}  // namespace warhol
