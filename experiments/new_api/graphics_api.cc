// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <thread>

#include <warhol/assets/asset_paths.h>
#include <warhol/graphics/graphics.h>
#include <warhol/input/input.h>
#include <warhol/memory/memory_pool.h>
#include <warhol/memory/memory_tracker.h>
#include <warhol/platform/timing.h>
#include <warhol/scene/camera.h>
#include <warhol/ui/imgui.h>
#include <warhol/utils/glm_impl.h>
#include <warhol/utils/log.h>
#include <warhol/window/window.h>

#include "game.h"

using namespace warhol;
using namespace warhol::imgui;

int main() {
  Game game = {};
  if (!InitGame(&game, WindowBackendType::kSDLOpenGL, RendererType::kOpenGL)) {
    LOG(ERROR) << "Could not initialize game.";
    return 1;
  }

  LOG(DEBUG) << "Loading shaders.";

  // Start pushing rendering actions.
  MemoryPool memory_pool;
  memory_pool.name = "Main";
  InitMemoryPool(&memory_pool, MEGABYTES(1));

  Track(&game.memory_tracker, &memory_pool);

  LOG(DEBUG) << "Setting camera.";

  Camera camera;
  camera.view = glm::lookAt(glm::vec3{2.0f, 2.0f, 2.0f}, {}, {0, 0, 0.1f});
  camera.projection =
      glm::perspective(glm::radians(45.0f),
                       (float)game.window.width / (float)game.window.height,
                       0.1f, 100.f);

  LOG(DEBUG) << "Setting drawer.";

  Drawer drawer;
  InitDrawer(&drawer, &window);

  LOG(DEBUG) << "Staring game loop.";

  Vec3 delta;
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

    delta.x += 0.1f * game.time.frame_delta;
    delta.y += 0.2f * game.time.frame_delta;
    delta.z += 0.05f * game.time.frame_delta;
    /* game.renderer.clear_color = delta; */

    ResetMemoryPool(&memory_pool);

    int sq = 10;  // square width.
    int b = 2;    // border.
    int y = 0;
    while (y < game.window.height) {
      int x = 0;

      while (x < game.window.width) {
        float u = 0.5f - ((float)x / (float)game.window.width);
        float v = 0.5f - ((float)y / (float)game.window.height);

        Vec3 color = {};

        float r = 0.5f + 0.5f* sin(sqrt(u*u + v*v) * 25 * game.time.seconds);
        color.x = r * sin(delta.x);
        color.y = r * sin(delta.y);
        color.z = r * sin(delta.z);

        DrawSquare(&game.drawer, {x + b, y + b}, {x + sq - b, y + sq - b},
                   color);


        x += sq;
      }
      y += sq;
    }

    auto command_list = CreateList<RenderCommand>(&memory_pool);
    Push(&command_list, ImguiEndFrame(&game.imgui));
    Push(&command_list, DrawerEndFrame(&game.drawer));

    EndFrame(&game, std::move(command_list));
  }

  LOG(DEBUG) << "Adios, amigo!";
}
