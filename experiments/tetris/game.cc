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

  LOG(DEBUG) << "Initializing imgui.";

  if (!InitImgui(&game->renderer, &game->imgui)) {
    LOG(ERROR) << "Could not start imgui.";
    return false;
  }
  TrackImguiMemory(&game->memory_tracker, &game->imgui);

  game->input = InputState::Create();

  if (!InitDrawer(&game->drawer, &game->renderer, &game->window)) {
    LOG(ERROR) << "Could not start drawer.";
    return false;
  }

  return true;
}

List<WindowEvent> NewFrame(Game* game) {
  PlatformUpdateTiming(&game->time);
  ImguiStartFrame(&game->window, &game->time, &game->input, &game->imgui);
  DrawerNewFrame(&game->drawer);
  return UpdateWindow(&game->window, &game->input);
}

void EndFrame(Game* game, List<RenderCommand> command_list) {
  RendererStartFrame(&game->renderer);
  RendererExecuteCommands(&game->renderer, &command_list);
  RendererEndFrame(&game->renderer);
  WindowSwapBuffers(&game->window);
}

void CreateImguiUI(Game* game) {
  ImGui::Begin("NEW API");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f * game->time.frame_delta_average,
              game->time.frame_rate);

  ImGui::Separator();

  ImGui::InputInt2("Mouse Whell", (int*)&game->input.mouse.wheel);

  ImGui::Separator();

  for (MemoryPool* pool : game->memory_tracker.tracked_pools) {
    float used_ratio = (float)Used(pool) / (float)pool->size;
    auto used_str = BytesToString(Used(pool));
    auto total_str = BytesToString(pool->size);
    auto bar = StringPrintf("%s/%s", used_str.c_str(), total_str.c_str());

    ImGui::ProgressBar(used_ratio, {0, 0}, bar.c_str());
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("%s", pool->name);
  }

  for (Mesh* mesh : game->memory_tracker.tracked_meshes) {
    {
      MemoryPool* pool = &mesh->vertices;
      float used_ratio = (float)Used(pool) / (float)pool->size;
      auto used_str = BytesToString(Used(pool));
      auto total_str = BytesToString(pool->size);
      auto bar = StringPrintf("%s/%s", used_str.c_str(), total_str.c_str());

      ImGui::ProgressBar(used_ratio, {0, 0}, bar.c_str());

      auto pool_name = StringPrintf("%s.%s", mesh->name, pool->name);
      ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
      ImGui::Text("%s", pool_name.c_str());
    }

    {
      MemoryPool* pool = &mesh->indices;
      float used_ratio = (float)Used(pool) / (float)pool->size;
      auto used_str = BytesToString(Used(pool));
      auto total_str = BytesToString(pool->size);
      auto bar = StringPrintf("%s/%s", used_str.c_str(), total_str.c_str());

      ImGui::ProgressBar(used_ratio, {0, 0}, bar.c_str());

      auto pool_name = StringPrintf("%s.%s", mesh->name, pool->name);
      ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
      ImGui::Text("%s", pool_name.c_str());
    }
  }

  ImGui::End();
}

}  // namespace tetris

