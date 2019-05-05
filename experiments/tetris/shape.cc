// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shape.h"

#include <warhol/utils/log.h>

namespace tetris {

Shape::Shape(const char* name, std::vector<Int2> offsets)
    : name(name), offsets(offsets), rotated_offsets(std::move(offsets)) {}

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

Collision
CheckShapeCollision(Board* board, Shape* shape, Int2 pivot) {
  return CheckCollision(board, pivot, shape->rotated_offsets);
}


Collision
CheckCollision(Board* board, Int2 pivot, const std::vector<Int2>& offsets) {
  for (const Int2& sqr_offset : offsets) {
    Int2 sqr_pos = pivot + sqr_offset;
    if (sqr_pos.y >= board->height)
      continue;

    if (sqr_pos.y < 0)
      return {CollisionType::kBottom, sqr_pos};

    if (!WithinBoundsX(board, sqr_pos.x))
      return {CollisionType::kBorder, sqr_pos};

    // Check if we hit a shape.
    uint8_t sqr = GetSquare(board, sqr_pos);
    if (sqr == kDeadBlock)
      return {CollisionType::kShape, sqr_pos};
  }

  return {};
}

// Utils -----------------------------------------------------------------------

std::vector<Int2> GetRotatedOffsets(Shape* shape, int index) {
  // We apply rotation matrices for all "right" angles (0, 90, 180, 270).
  // Note that these angles grow clockwise.
  static IntMat2 rotations[4] = {
    IntMat2::FromRows({ 1,  0}, { 0,  1}),  // 0.
    IntMat2::FromRows({ 0,  1}, {-1,  0}),  // pi / 4.
    IntMat2::FromRows({-1,  0}, { 0, -1}),  // pi / 2.
    IntMat2::FromRows({ 0, -1}, { 1,  0}),  // (3 * pi) / 4.

  };

  index = index % 4;
  std::vector<Int2> offsets;
  offsets.reserve(shape->offsets.size());
  for (auto& offset : shape->offsets) {
    offsets.push_back(rotations[index] * offset);
  }

  return offsets;
}

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
  ASSERT(index >= 0) << "X: " << x << ", Y: " << y;
  ASSERT(true);
  return board->_slots[index];
}

void SetSquare(Board* board, uint8_t val, int x, int y) {
  int index = CoordToIndex(board, x, y);
  ASSERT(index >= 0) << "X: " << x << ", Y: " << y;
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

IntBox2 GetShapeBoundingBox(Shape* shape, Int2 pos) {
  IntBox2 bounds;
  bounds.min = {10000, -10000};
  bounds.max = {10000, -10000};
  for (Int2 offset : shape->offsets) {
    Int2 true_pos = pos + offset;
    if (true_pos.x < bounds.min.x) bounds.min.x = true_pos.x;
    if (true_pos.x > bounds.min.y) bounds.min.y = true_pos.x;

    if (true_pos.y < bounds.max.x) bounds.max.x = true_pos.y;
    if (true_pos.y > bounds.max.y) bounds.max.y = true_pos.y;
  }

  return bounds;
}

const char* CollisionTypeToString(CollisionType type) {
  switch (type) {
    case CollisionType::kNone: return "None";
    case CollisionType::kBorder: return "Border";
    case CollisionType::kBottom: return "Bottom";
    case CollisionType::kShape: return "Shape";
  }

  NOT_REACHED();
  return nullptr;
}

}  // namespace tetris
