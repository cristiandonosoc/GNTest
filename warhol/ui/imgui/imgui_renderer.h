// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/common/shader.h"
#include "warhol/graphics/common/texture.h"
#include "warhol/graphics/common/mesh.h"
#include "warhol/utils/macros.h"

struct ImGuiIO;

namespace warhol {

struct Renderer;

namespace imgui {

struct ImguiRenderer {
  RAII_CONSTRUCTORS(ImguiRenderer);

  Mesh mesh;
  Shader shader;
  Texture font_texture;

  ImGuiIO* io = nullptr;
  Renderer* renderer = nullptr;   // Must outlive.
};

inline bool Valid(ImguiRenderer* r) { return !!r->renderer && !!r->io; }

// Requires ImGuiRenderer.io to be already set.
bool InitImguiRenderer(Renderer*, ImguiRenderer*);
void ShutdownImguiRenderer(ImguiRenderer*);

}  // namespace imgui
}  // namespace warhol

