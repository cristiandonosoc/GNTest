// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "imgui.h"

#include <warhol/ui/imgui.h>

#include "game.h"
#include "tetris.h"
#include "tetris_renderer.h"

namespace tetris {

namespace {

void SystemUI(Game*, Tetris*);
void TetrisUI(Game*, Tetris*);

}  // namespace

void DoImguiUI(Game* game, Tetris* tetris) {
  SCOPE_LOCATION();

  ImguiStartFrame(&game->window, &game->time, &game->input, &game->imgui);

  static bool show_system_ui = true;
  if (KeyDownThisFrame(&game->input, Key::kBackquote))
    show_system_ui = !show_system_ui;

  if (show_system_ui) {
    SystemUI(game, tetris);
    TetrisUI(game, tetris);
  }
}

namespace {

void SystemUI(Game* game, Tetris* tetris) {
  auto dims = GetTetrisScreenDimensions(game, tetris);
  ImGui::SetNextWindowPos({0, 0});
  ImGui::SetNextWindowSize({(float)dims.screen_pad - 3.0f,
                            (float)game->window.height});

  ImGui::Begin("System Info");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f * game->time.frame_delta_average,
              game->time.frame_rate);

  ImGui::Separator();

  if (ImGui::CollapsingHeader("Global State", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Checkbox("No auto down", &tetris->no_down);
  }

  if (ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen)) {
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
  }

  ImGui::End();
}

void TetrisUI(Game* game, Tetris* tetris) {
  auto dims = GetTetrisScreenDimensions(game, tetris);
  ImGui::SetNextWindowPos({dims.screen_pad + dims.board_width + 3.0f, 0.0f});
  ImGui::SetNextWindowSize({(float)dims.screen_pad,
                            (float)game->window.height});

  ImGui::Begin("Tetris");

  if (ImGui::CollapsingHeader("Current Shape",
                              ImGuiTreeNodeFlags_DefaultOpen)) {


    ImGui::LabelText("Name", "%s", tetris->current_shape.shape.name);
    ImGui::InputFloat2("Pos", (float*)&tetris->current_shape.pos);
    ImGui::InputInt("Rotation", &tetris->current_shape.shape.rotation);

    Shape* shape = &tetris->current_shape.shape;
    if (Valid(shape)) {
      IntMat2* rotation_matrix = GetRotationMatrix(shape);
      auto rows = ToRowArray(rotation_matrix);
      ImGui::InputInt2("row1", (int*)&rows);
      ImGui::InputInt2("row2", (int*)&rows[2]);
    }
  }

  if (ImGui::CollapsingHeader("Timings", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::ProgressBar(TimerRatio(game, &tetris->auto_down_tick), {});
    ImGui::SameLine(); ImGui::Text("%s", "Auto down");
    ImGui::ProgressBar(TimerRatio(game, &tetris->next_shape_tick), {});
    ImGui::SameLine(); ImGui::Text("%s", "Next shape");
    ImGui::ProgressBar(TimerRatio(game, &tetris->side_move_tick), {});
    ImGui::SameLine(); ImGui::Text("%s", "Side move");
    ImGui::ProgressBar(TimerRatio(game, &tetris->down_move_tick), {});
    ImGui::SameLine(); ImGui::Text("%s", "Down move");
  }

  ImGui::End();
}

}  // namespace

}  // namespace tetris
