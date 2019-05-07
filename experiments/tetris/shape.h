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
constexpr uint8_t kPivot = 3;       // Represents the pivot point of a shape.
constexpr uint8_t kShadow = 4;      // Where the shape will be when dropped.

// The offsets define the places where this shape has a square offseted from its
// position.
struct Shape {
  Shape() = default;
  Shape(const char* name,
        std::vector<Int2> offsets,
        std::vector<IntMat2*> rotation_matrices);

  const char* name = nullptr;
  int rotation = 0;   // +1 means a clockwise rotation.
  std::vector<Int2> offsets;
  std::vector<IntMat2*> rotation_matrices;

  // Cache of offsets * current rotation_matrix.
  std::vector<Int2> rotated_offsets;
};

inline bool Valid(Shape* shape) { return !shape->offsets.empty(); }
// INT_MIN means the shape default.
IntMat2* GetRotationMatrix(Shape* shape, int index = INT_MIN);

const std::vector<Shape>& GetShapes();
Shape GetRandomShape();

// Board -----------------------------------------------------------------------

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
  kSame,      // Used for shadow placement.
};
const char* CollisionTypeToString(CollisionType);

// Checks if where a shape would move creates some kind of collision.
struct Collision {
  CollisionType type = CollisionType::kNone;
  Int2 pos;       // Where the collision occured.
};
Collision CheckShapeCollision(Board*, Shape*, Int2 pivot);

// If |collide_live| is true, check if the offsets collides with a live shape
// placed within the board.
Collision CheckCollision(Board*, Int2 pivot,
                         const std::vector<Int2>& offsets,
                         bool collide_live = false);

// Utils -----------------------------------------------------------------------

std::vector<Int2> GetRotatedOffsets(Shape*, int offset);

uint8_t GetSquare(Board*, Int2 coord);
uint8_t GetSquare(Board*, int x, int y);
void SetSquare(Board*, uint8_t val, int x, int y);
void SetSquare(Board*, uint8_t val, Int2 coord);

int CoordToIndex(Board*, Int2 coord);
int CoordToIndex(Board*, int x, int y);

bool WithinBounds(Board*, Int2 coord);
bool WithinBounds(Board*, int x, int y);

IntBox2 GetShapeBoundingBox(Shape*, Int2);

}  // namespace tetris
