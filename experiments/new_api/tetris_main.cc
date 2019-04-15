// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <thread>

#include <warhol/graphics/graphics.h>
#include <warhol/memory/memory_pool.h>
#include <warhol/ui/imgui.h>
#include <warhol/utils/log.h>
#include <warhol/window/window.h>

#include "game.h"
#include "tetris.h"

using namespace warhol;
using namespace warhol::imgui;
using namespace tetris;

int main() {
  Game game = {};
  if (!InitGame(&game, WindowBackendType::kSDLOpenGL, RendererType::kOpenGL)) {
    LOG(ERROR) << "Could not initialize game.";
    return 1;
  }

  MemoryPool memory_pool;
  memory_pool.name = "Main";
  InitMemoryPool(&memory_pool, MEGABYTES(1));

  Tetris tetris;
  if (!InitTetris(&tetris)) {
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

    if (!running || game.input.keys_up[GET_KEY(Escape)])
      break;

    ResetMemoryPool(&memory_pool);

    UpdateTetris(&game, &tetris);



    auto command_list = CreateList<RenderCommand>(&memory_pool);
    /* Push(&command_list, TetrisEndFrame(&game, &tetris)); */
    Push(&command_list, ImguiEndFrame(&game.imgui));

    EndFrame(&game, std::move(command_list));
  }

  LOG(DEBUG) << "Adios, amigo!";
}
