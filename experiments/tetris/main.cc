// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <warhol/graphics/graphics.h>
#include <warhol/memory/memory_pool.h>
#include <warhol/ui/imgui.h>
#include <warhol/utils/log.h>
#include <warhol/window/window.h>

#include <thread>

#include "imgui.h"
#include "game.h"
#include "tetris.h"

using namespace warhol;
using namespace warhol::imgui;
using namespace tetris;

int main() {
  SCOPE_LOCATION();

  Game game = {};
  if (!InitGame(&game, WindowBackendType::kSDLOpenGL, RendererType::kOpenGL)) {
    LOG(ERROR) << "Could not initialize game.";
    return 1;
  }

  MemoryPool memory_pool;
  memory_pool.name = "Main";
  InitMemoryPool(&memory_pool, MEGABYTES(1));

  Tetris tetris;
  if (!InitTetris(&game, &tetris)) {
    LOG(ERROR) << "Could not initialize tetris.";
    return 1;
  }

  Track(&game.memory_tracker, &memory_pool);

  LOG(DEBUG) << "Staring game loop.";

  bool running = true;
  while (running) {
    auto events = NewFrame(&game);
    for (WindowEvent event : events) {
      if (event == WindowEvent::kQuit) {
        running = false;
        break;
      }
    }

    if (!running || KeyUpThisFrame(&game.input, Key::kEscape))
      break;

    ResetMemoryPool(&memory_pool);

    TetrisNewFrame(&game, &tetris);
    /* DoImguiUI(&game, &tetris); */

    auto command_list = CreateList<RenderCommand>(KILOBYTES(16));

    Push(&command_list, TetrisEndFrame(&game, &tetris));
    /* Push(&command_list, ImguiEndFrame(&game.imgui)); */

    EndFrame(&game, std::move(command_list));
  }

  printf("またね\n");
}
