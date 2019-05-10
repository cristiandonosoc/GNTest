// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <warhol/graphics/graphics.h>
#include <warhol/scene/camera.h>

using namespace warhol;

namespace tetris {

struct Game;

struct Colors {
  static constexpr uint32_t kBlack=   0xff'00'00'00;
  static constexpr uint32_t kBlue=    0xff'ff'00'00;
  static constexpr uint32_t kGreen =  0xff'00'ff'00;
  static constexpr uint32_t kRed =    0xff'00'00'ff;
  static constexpr uint32_t kWhite = 0xff'ff'ff'ff;
  static constexpr uint32_t kTeal = 0xff'f9'f0'ea;
  static constexpr uint32_t kGray = 0xff'99'99'99;
};

struct Drawer {
  RAII_CONSTRUCTORS(Drawer);

  Camera camera;
  Mesh mesh;
  Shader shader;

  MemoryPool pool;

  // Must outlive.
  Renderer* renderer = nullptr;
  Window* window = nullptr;
};

bool Valid(Drawer*);

bool InitDrawer(Game*, Drawer* out);

void ShutdownDrawer(Drawer*);

void DrawerNewFrame(Drawer*);

void DrawSquare(Drawer*, Int2 tl, Int2 br, uint32_t color);
void DrawBorderSquare(Drawer*, Int2 tl, Int2 br, uint32_t color);

::warhol::RenderCommand DrawerEndFrame(Drawer*);

}  // namespace tetris