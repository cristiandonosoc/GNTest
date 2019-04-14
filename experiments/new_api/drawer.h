// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <warhol/graphics/graphics.h>
#include <warhol/scene/camera.h>

using namespace warhol;

struct Colors {
  static constexpr uint32_t kBlack=   0xff'00'00'00;
  static constexpr uint32_t kBlue=    0xff'ff'00'00;
  static constexpr uint32_t kGreen =  0xff'00'ff'00;
  static constexpr uint32_t kRed =    0xff'00'00'ff;
  static constexpr uint32_t kWhite =  0xff'ff'ff'ff;
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

bool InitDrawer(Drawer*, Renderer*, Window*);

void ShutdownDrawer(Drawer*);

void DrawerNewFrame(Drawer*);

void DrawSquare(Drawer*, Pair<int> bl, Pair<int> tr, Vec3 color);

::warhol::RenderCommand DrawerEndFrame(Drawer*);

