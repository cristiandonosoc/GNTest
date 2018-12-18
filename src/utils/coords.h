// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <assert.h>
#include <stdlib.h>

#include "src/math/vec.h"

namespace warhol {

inline int
Coord2ToArrayIndex(int side, int x, int y) {
  if (x < 0 || x >= side || y < 0 || y >= side)
    return -1;
  return x + y * side;
}

inline int
Coord3ToArrayIndex(int side, int x, int y, int z) {
  if (x < 0 || x >= side || y < 0 || y >= side || z < 0 || z >= side)
    return -1;
  int z_offset = z * side;
  int y_offset = y * side * side;
  return x + y_offset + z_offset;
}

inline Pair<int>
ArrayIndexToCoord2(int side, int index) {
  Pair<int> coord = {};
  coord.y = index/ side;
  coord.x = index % side;
  return coord;
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
