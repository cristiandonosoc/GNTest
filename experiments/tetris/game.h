// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <warhol/platform/timing.h>
#include <warhol/window/window.h>
#include <warhol/graphics/graphics.h>
#include <warhol/ui/imgui.h>

#include <warhol/input/input.h>

#include "drawer.h"

using namespace warhol;

namespace tetris {

struct Game {
  ::warhol::PlatformTime time;
  ::warhol::MemoryTracker memory_tracker;
  ::warhol::Window window;
  ::warhol::Renderer renderer;
  ::warhol::InputState input;

  ::warhol::imgui::ImguiContext imgui;

  Drawer drawer;
};

bool InitGame(Game*, WindowBackendType, RendererType);
List<WindowEvent> NewFrame(Game*);
void EndFrame(Game*, List<RenderCommand>);

}  // namespace tetris
