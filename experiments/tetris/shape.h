// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include <warhol/math/vec.h>

using namespace warhol;

namespace tetris {

// The offsets define the places where this shape has a square offseted from its
// position.
struct Shape {
  std::vector<Int2> offsets;
};

inline bool Valid(Shape* shape) { return !shape->offsets.empty(); }

/* constexpr uint8_t kDeadBlock = 1;   // A block that is already stationed. */
constexpr uint8_t kLiveBlock = 2;   // A block of a current shape.

struct Board {
  uint32_t width = 0;
  uint32_t height = 0;

  std::vector<uint8_t> slots;
};

inline bool Valid(Board* board) { return !board->slots.empty(); }

}  // namespace tetris
