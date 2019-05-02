// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "imgui.h"

#include <warhol/ui/imgui.h>

#include "game.h"
#include "tetris.h"
#include "tetris_renderer.h"

namespace tetris {

void DoImguiUI(Game* game, Tetris* tetris) {
  auto dims = GetTetrisScreenDimensions(game, tetris);
  ImGui::SetNextWindowPos({dims.screen_pad + dims.board_width + 3.0f, 0.0f});
  ImGui::SetNextWindowSize({(float)dims.screen_pad,
                            (float)game->window.height});

  ImGui::Begin("NEW API");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f * game->time.frame_delta_average,
              game->time.frame_rate);


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
