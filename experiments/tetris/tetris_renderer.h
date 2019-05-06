// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <warhol/graphics/graphics.h>
#include <warhol/scene/camera.h>

using namespace warhol;

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

struct TetrisRenderer {
  RAII_CONSTRUCTORS(TetrisRenderer);

  Camera camera;
  Mesh mesh;
  Shader shader;
  MemoryPool pool;

  // Must outlive.
  Renderer* renderer = nullptr;
  Window* window = nullptr;
};
bool Valid(TetrisRenderer*);

bool Init(TetrisRenderer*, Renderer*, Window*);
void Shutdown(TetrisRenderer*);


void NewFrame(TetrisRenderer*);
bool EndFrame(TetrisRenderer*, List<RenderCommand>* out);

::warhol::RenderCommand GetTetrisRenderCommand(Game*, Tetris*);


}  // namespace tetris
