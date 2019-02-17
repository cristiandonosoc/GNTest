// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <memory>
#include <vector>

#include "warhol/utils/macros.h"

#include "warhol/graphics/common/renderer_backend.h"

namespace warhol {

struct Mesh;
struct WindowManager;

struct RenderCommand {
  enum class Type {
    kRenderMesh,
    kLast,
  };
  static const char* TypeToString(Type);

  Type type = Type::kLast;
  Mesh* mesh;       // Not owning. Must outlive.
  Camera* camera;   // Not owning. Must outlive.
};

struct Renderer {
  bool valid() const { return window != nullptr && backend.valid(); }
  RendererBackend::Type backend_type() const { return backend.type; }
  RendererBackend::Interface& interface() { return backend.interface; }

  Renderer();
  ~Renderer();
  DELETE_COPY_AND_ASSIGN(Renderer);
  DELETE_MOVE_AND_ASSIGN(Renderer);

  WindowManager* window = nullptr;
  RendererBackend backend = {};

  std::vector<RenderCommand> render_commands;
};

bool InitRenderer(Renderer*, RendererBackend::Type);
void ShutdownRenderer(Renderer*);

void WindowSizeChanged(Renderer*, uint32_t width, uint32_t height);
bool DrawFrame(Renderer*, Camera*);

}  // namespace
