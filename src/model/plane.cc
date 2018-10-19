// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/model/plane.h"

#include <math.h>

namespace warhol {

std::vector<float> Plane::Create(size_t width, size_t length) {
  float fwidth = floor(width) / 2;
  float flength = floor(length) / 2;

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
