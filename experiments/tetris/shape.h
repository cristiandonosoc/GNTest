// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include <warhol/math/vec.h>
#include <warhol/utils/log.h>

using namespace warhol;

namespace tetris {

constexpr uint8_t kNone = 0;
constexpr uint8_t kDeadBlock = 1;   // A block that is already stationed.
constexpr uint8_t kLiveBlock = 2;   // A block of a current shape.

// The offsets define the places where this shape has a square offseted from its
// position.
struct Shape {
  std::vector<Int2> offsets;
};

inline bool Valid(Shape* shape) { return !shape->offsets.empty(); }

struct Board {
  int width = 0;
  int height = 0;

  std::vector<uint8_t> _slots;
};

inline bool Valid(Board* board) { return !board->_slots.empty(); }

enum class CollisionType {
  kNone,
  kBorder,
  kBottom,
  kShape,     // Hit another shape square.
};
const char* CollisionTypeToString(CollisionType);

// Checks if where a shape would move creates some kind of collision.
CollisionType CheckShapeCollision(Board*, Shape*, Int2 pos, Int2 offset);

// Utils -----------------------------------------------------------------------

uint8_t GetSquare(Board*, Int2 coord);
uint8_t GetSquare(Board*, int x, int y);
void SetSquare(Board*, uint8_t val, int x, int y);
void SetSquare(Board*, uint8_t val, Int2 coord);

int CoordToIndex(Board*, Int2 coord);
int CoordToIndex(Board*, int x, int y);

bool WithinBounds(Board*, Int2 coord);
bool WithinBounds(Board*, int x, int y);

}  // namespace tetris
