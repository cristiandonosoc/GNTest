// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/renderer.h"

#include "warhol/graphics/common/renderer_backend.h"
#include "warhol/utils/assert.h"
#include "warhol/window/window_manager.h"

namespace warhol {

// Renderer --------------------------------------------------------------------

Renderer::Renderer() = default;
Renderer::~Renderer() {
  if (valid())
    ShutdownRenderer(this);
}

void InitRenderer(Renderer* renderer, RendererBackend::Type type) {
  ASSERT(type != RendererBackend::Type::kLast);

  renderer->backend = CreateRendererBackend(type);
  renderer->backend->Init(renderer);
}

// An null backend renderer can happen if Shutdown was called before the
// destructor.
void ShutdownRenderer(Renderer* renderer) {
  ASSERT(renderer->valid());
  renderer->backend->Shutdown();
}

// void WindowSizeChanged(Renderer* renderer, uint32_t width, uint32_t height) {}

void DrawFrame(Renderer* renderer, Camera* camera) {
  ASSERT(renderer->valid());
  renderer->backend->DrawFrame(camera);
}

}  // namespace warhol
