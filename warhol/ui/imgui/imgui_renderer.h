// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/common/shader.h"
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
  Renderer* renderer = nullptr;
};

inline bool Valid(ImguiRenderer* r) { return !!r->renderer; }

bool InitImguiRenderer(Renderer*, ImguiRenderer*);
void ShutdownImguiRenderer(ImguiRenderer*);

}  // namespace imgui
}  // namespace warhol

