// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/containers/linked_list.h"
#include "warhol/graphics/common/shader.h"
#include "warhol/graphics/common/texture.h"
#include "warhol/graphics/common/mesh.h"
#include "warhol/scene/camera.h"
#include "warhol/utils/macros.h"

struct ImGuiIO;

namespace warhol {

struct Renderer;
struct MeshRenderAction;

namespace imgui {

struct ImguiContext;

struct ImguiRenderer {
  RAII_CONSTRUCTORS(ImguiRenderer);

  Camera camera;

  Mesh mesh;
  Shader shader;
  Texture font_texture;

  ImGuiIO* io = nullptr;
  Renderer* renderer = nullptr;   // Must outlive.

  MemoryPool memory_pool;
};

inline bool Valid(ImguiRenderer* r) { return !!r->renderer && !!r->io; }

// Requires ImGuiRenderer.io to be already set.
bool InitImguiRenderer(ImguiRenderer*, Renderer*);
void ShutdownImguiRenderer(ImguiRenderer*);

}  // namespace imgui
}  // namespace warhol

