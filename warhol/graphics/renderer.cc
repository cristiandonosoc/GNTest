// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/renderer.h"

#include "warhol/utils/assert.h"
#include "warhol/window/window_manager.h"

namespace warhol {

// RenderCommand ---------------------------------------------------------------

// Renderer --------------------------------------------------------------------

Renderer::Renderer() = default;
Renderer::~Renderer() {
  if (valid())
    ShutdownRenderer(this);
}

void InitRenderer(Renderer* renderer, RendererBackend::Type type) {
  ASSERT(renderer->window->backend.type != WindowManagerBackend::Type::kLast);
  ASSERT(type != RendererBackend::Type::kLast);

  renderer->backend = GetRendererBackend(type);
  renderer->backend.renderer = renderer;  // Set the back pointer.

  // Initialize the backend.
  auto& interface = renderer->backend.interface;
  interface.Init(&renderer->backend);
}

// An null backend renderer can happen if Shutdown was called before the
// destructor.
void ShutdownRenderer(Renderer* renderer) {
  ASSERT(renderer->valid());
  auto& interface = renderer->interface();
  interface.Shutdown(&renderer->backend);
}

// void WindowSizeChanged(Renderer* renderer, uint32_t width, uint32_t height) {}

void DrawFrame(Renderer* renderer, Camera* camera) {
  ASSERT(renderer->valid());
  auto& interface = renderer->interface();
  interface.DrawFrame(&renderer->backend, camera);
}

}  // namespace warhol
