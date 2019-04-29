// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shape.h"

#include <warhol/utils/log.h>

namespace tetris {

namespace {

bool WithinBoundsX(Board* board, int x) {
  if (x < 0 || x >= board->width)
    return false;
  return true;
}

bool WithinBoundsY(Board* board, int y) {
  if (y < 0 || y >= board->height)
    return false;
  return true;
}


}  // namespace

CollisionType
CheckShapeCollision(Board* board, Shape* shape, Int2 pos, Int2 offset) {
  SCOPE_LOCATION();
  if (pos == Int2::Zero())
    LOG(DEBUG) << "Checking zero pos";
  for (Int2& sqr_offset : shape->offsets) {
    Int2 sqr_pos = pos + sqr_offset + offset;
    if (sqr_pos.y < 0)
      return CollisionType::kBottom;

    if (!WithinBoundsX(board, sqr_pos.x))
      return CollisionType::kBorder;

    // Check if we hit a shape.
    uint8_t sqr = GetSquare(board, sqr_pos);
    if (sqr == kDeadBlock)
      return CollisionType::kShape;
  }

  return CollisionType::kNone;
}

// Utils -----------------------------------------------------------------------

int CoordToIndex(Board* board, Int2 coord) {
  return CoordToIndex(board, coord.x, coord.y);
}

int CoordToIndex(Board* board, int x, int y) {
  if (x < 0 || x >= board->width || y < 0 || y >= board->height)
    return -1;
  return y * board->width + x;
}

uint8_t GetSquare(Board* board, Int2 pos) {
  return GetSquare(board, pos.x, pos.y);
}

uint8_t GetSquare(Board* board, int x, int y) {
  int index = CoordToIndex(board, x, y);
  ASSERT(index >= 0);
  return board->_slots[index];
}

void SetSquare(Board* board, uint8_t val, int x, int y) {
  int index = CoordToIndex(board, x, y);
  ASSERT(index >= 0);
  board->_slots[index] = val;
}

void SetSquare(Board* board, uint8_t val, Int2 coord) {
  return SetSquare(board, val, coord.x, coord.y);
}

bool WithinBounds(Board* board, Int2 pos) {
  return WithinBounds(board, pos.x, pos.y);
}

bool WithinBounds(Board* board, int x, int y) {
  return WithinBoundsX(board, x) && WithinBoundsY(board, y);
}

const char* CollisionTypeToString(CollisionType type) {
  switch (type) {
    case CollisionType::kNone: return "None";
    case CollisionType::kBorder: return "Border";
    case CollisionType::kBottom: return "Bottom";
    case CollisionType::kShape: return "Shape";
  }

  NOT_REACHED("Invalid collision type");
  return nullptr;
}

}  // namespace tetris
