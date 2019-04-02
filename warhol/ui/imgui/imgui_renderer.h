// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <third_party/imgui/imgui.h>

#include "warhol/graphics/common/shader.h"
#include "warhol/graphics/common/texture.h"
#include "warhol/graphics/common/unstructured_buffer.h"
#include "warhol/utils/macros.h"

namespace warhol {

struct Renderer;

namespace imgui {

struct ImguiRenderer {
  ImguiRenderer() = default;
  ~ImguiRenderer();   // RAII
  DELETE_COPY_AND_ASSIGN(ImguiRenderer);
  DEFAULT_MOVE_AND_ASSIGN(ImguiRenderer);

  Shader shader;
  Texture font_texture;
  UnstructuredBuffer buffer;

  ImGuiIO* io = nullptr;
  Renderer* renderer = nullptr;   // Must outlive.
};

inline bool Valid(ImguiRenderer* r) { return !!r->renderer && !!r->io; }

bool InitImguiRenderer(Renderer*, ImguiRenderer*);
void ShutdownImguiRenderer(ImguiRenderer*);

}  // namespace imgui
}  // namespace warhol

