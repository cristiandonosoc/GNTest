// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <memory>
#include <vector>

#include "warhol/utils/macros.h"

#include "warhol/graphics/common/render_command.h"
#include "warhol/graphics/common/renderer_backend.h"

namespace warhol {

struct RendererBackend;
struct WindowManager;

struct Renderer {
  Renderer();
  ~Renderer();
  DELETE_COPY_AND_ASSIGN(Renderer);
  DELETE_MOVE_AND_ASSIGN(Renderer);

  bool valid() const { return backend && backend->valid(); }

  WindowManager* window = nullptr;
  std::unique_ptr<RendererBackend> backend = {};

  std::vector<RenderCommand> render_commands;
};

void InitRenderer(Renderer*, RendererBackend::Type);
void ShutdownRenderer(Renderer*);

void WindowSizeChanged(Renderer*, uint32_t width, uint32_t height);
void DrawFrame(Renderer*, Camera*);

}  // namespace
