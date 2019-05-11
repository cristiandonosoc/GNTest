// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "game.h"

using namespace warhol;
using namespace warhol::imgui;

namespace tetris {

bool InitGame(Game* game,
              ::warhol::WindowBackendType window_type,
              ::warhol::RendererType renderer_type) {

  InitWindowConfig window_config;
  /* window_config.maximized = true; */
  window_config.resizable = true;
  if (!InitWindow(&game->window, window_type, &window_config)) {
    LOG(ERROR) << "Could not start SDL.";
    return false;
  }

  LOG(DEBUG) << "Initializing renderer.";

  if (!InitRenderer(&game->renderer, renderer_type, &game->window)) {
    LOG(ERROR) << "Could not start renderer.";
    return false;
  }
  game->paths = GetBasePaths(game->renderer.type);

  LOG(DEBUG) << "Initializing imgui.";

  if (!InitImgui(&game->renderer, &game->imgui)) {
    LOG(ERROR) << "Could not start imgui.";
    return false;
  }
  TrackImguiMemory(&game->memory_tracker, &game->imgui);

  game->input = InputState::Create();

  if (!InitDrawer(game, &game->drawer)) {
    LOG(ERROR) << "Could not start drawer.";
    return false;
  }

  return true;
}

List<WindowEvent> NewFrame(Game* game) {
  PlatformUpdateTiming(&game->time);
  DrawerNewFrame(&game->drawer);
  return UpdateWindow(&game->window, &game->input);
}

void EndFrame(Game* game, List<RenderCommand> command_list) {
  SCOPE_LOCATION();

  RendererStartFrame(&game->renderer);
  RendererExecuteCommands(&game->renderer, &command_list);
  RendererEndFrame(&game->renderer);
  WindowSwapBuffers(&game->window);
}

}  // namespace tetris

