// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <assert.h>
#include <stdlib.h>

#include "src/math/vec.h"

namespace warhol {

// Coords are like this in the array:
// YZXXXZXXXYZXXXZXXX

inline size_t
Coord3ToArrayIndex(size_t side, size_t x, size_t y, size_t z) {
  assert(x < side && y < side && z < side);
  size_t z_offset = z * side;
  size_t y_offset = y * side * side;
  return x + y_offset + z_offset;
}

inline Pair3<size_t>
ArrayIndexToCoord3(size_t side, size_t index) {
  assert(index < side * side * side);

  Pair3<size_t> coord = {};
  size_t square_side = side * side;
  coord.y = index / square_side;
  index -= coord.z * square_side;
  coord.z = index / side;
  coord.x = index % side;

  return coord;
}

}  // namespace warhol
