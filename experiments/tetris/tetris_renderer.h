// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <warhol/graphics/graphics.h>

namespace tetris {

struct Game;
struct Tetris;

// How much to the left the UI is.

struct TetrisScreenDimensions {
  int block_size;     // How big a block is in pixels.
  int board_width;    // Width of the board in pixels.
  int screen_pad;     // How much padding (border) the board if offset (pixesl).
};

TetrisScreenDimensions GetTetrisScreenDimensions(Game*, Tetris*);

::warhol::RenderCommand GetTetrisRenderCommand(Game*, Tetris*);

}  // namespace tetris
