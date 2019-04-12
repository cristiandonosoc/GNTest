// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <warhol/memory/memory_pool.h>
#include <warhol/graphics/graphics.h>
#include <warhol/scene/camera.h>

#include <warhol/math/vec.h>

namespace warhol {

struct Window;

}  // namespace warhol


struct DrawVertex {
  float pos[2];
  uint8_t color[4];
};

struct Square {
  ::warhol::Pair<int> bottom_left;
  ::warhol::Pair<int> top_right;

  uint8_t color[4];
};

struct Drawer {
  ::warhol::MemoryPool pool;

  ::warhol::Camera camera;
  ::warhol::Mesh mesh;
  ::warhol::Shader shader;

  ::warhol::Window* window = nullptr;   // Not-owning.

  uint32_t square_count = 0;

  bool valid_ = false;
};

inline bool Valid(Drawer* drawer) { return drawer->valid_; }

void InitDrawer(Drawer*, ::warhol::Window*);

void PushSquare(Drawer*, Square);

void DrawerStartFrame(Drawer*);

::warhol::RenderCommand DrawerEndFrame(Drawer*);

