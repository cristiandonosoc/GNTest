// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <warhol/math/vec.h>

#include "drawer.h"

struct Game;

namespace warhol {
struct RenderCommand;
}  // namespace warhol

namespace tetris {

enum class Move {
  kUp,
  kDown,
  kLeft,
  kRight,
};

enum class ShapeType {
  kSquare,
  kLast,    // Not meant to be a shape.
};

struct Shape {
  Move moves[3];
};

struct TetrisShape {
  ::warhol::Pair<int> pos;
  Shape* shape = nullptr;
};

struct Tetris {
  static constexpr int kWidth = 10;
  static constexpr int kHeight = 20;

  uint8_t board[kWidth * kHeight] = {};
  float move_tick = 1.0f;   // In seconds.
  float last_move_time = 0.0f;

  float next_shape_tick = 1.0f;
  float last_shape_time = 0.0f;

  TetrisShape current_shape = {};

  Drawer drawer;
};

// Define Shapes


struct Shapes {
  static Shape kSquare;
};

bool InitTetris(Game*, Tetris*);

void UpdateTetris(Game*, Tetris*);

::warhol::RenderCommand TetrisEndFrame(Game*, Tetris*);

}  // namespace tetris
