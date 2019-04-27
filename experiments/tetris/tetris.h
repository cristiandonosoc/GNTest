// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <warhol/math/vec.h>

#include "drawer.h"
#include "shape.h"

struct Game;

namespace tetris {

enum class ShapeType {
  kSquare,
  kLast,    // Not meant to be a shape.
};

struct TetrisShape {
  Int2 pos;
  Shape shape = {};
};

struct Tetris {
  Board board;

  float move_tick = 0.7f;   // In seconds.
  float last_move_time = 0.0f;

  float side_move_tick = 0.2f;
  float last_side_move = 0.0f;

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
