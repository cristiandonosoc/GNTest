// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <warhol/graphics/graphics.h>

namespace tetris {

struct Game;
struct Tetris;

::warhol::RenderCommand GetTetrisRenderCommand(Game*, Tetris*);

}  // namespace tetris
