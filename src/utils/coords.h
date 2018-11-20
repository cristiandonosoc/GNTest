// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <assert.h>
#include <stdlib.h>

#include "src/math/vec.h"

namespace warhol {

inline size_t
Coord2ToArrayIndex(size_t side, size_t x, size_t y) {
  return x + y * side;
}

inline size_t
Coord3ToArrayIndex(size_t side, size_t x, size_t y, size_t z) {
  size_t z_offset = z * side;
  size_t y_offset = y * side * side;
  return x + y_offset + z_offset;
}

inline Pair3<size_t>
ArrayIndexToCoord3(size_t side, size_t index) {
  Pair3<size_t> coord = {};
  size_t square_side = side * side;
  coord.y = index / square_side;
  index -= coord.z * square_side;
  coord.z = index / side;
  coord.x = index % side;

  return coord;
}

}  // namespace warhol
