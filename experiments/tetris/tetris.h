// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <warhol/math/vec.h>

#include "drawer.h"
#include "shape.h"

namespace tetris {

struct Game;

enum class ShapeType {
  kSquare,
  kLast,    // Not meant to be a shape.
};

struct TetrisShape {
  Int2 pos;
  Shape shape = {};
};

// Represents a timer that has to track when it last occured.
// Time time is in seconds.
struct TickTimer {
  float length;
  float last_tick;
};

// Will update |last_tick| if true.
bool HasTickTimerTriggered(Game*, TickTimer*);

struct TetrisStats {
  int lines_completed = 0;
};

struct Tetris {
  Board board;

  TickTimer next_shape_tick = {1.0f, 0.0f};
  TickTimer auto_down_tick = {0.7f, 0.0f};

  TickTimer side_move_tick = {0.2f, 0.0f};
  TickTimer down_move_tick = {0.05f, 0.0f};

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
