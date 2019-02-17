// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/renderer.h"

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"
#include "warhol/window/window_manager.h"

#ifdef WARHOL_VULKAN_ENABLED
#include "warhol/graphics/vulkan/renderer_backend.h"
#endif

namespace warhol {

// RenderCommand ---------------------------------------------------------------

const char* RenderCommand::TypeToString(RenderCommand::Type type) {
  switch (type) {
    case RenderCommand::Type::kRenderMesh: return "RenderMesh";
    case RenderCommand::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown RenderCommand::Type");
  return nullptr;
}

// Renderer --------------------------------------------------------------------

Renderer::Renderer() = default;
Renderer::~Renderer() {
  if (valid())
    ShutdownRenderer(this);
}

bool InitRenderer(Renderer* renderer, RendererBackend::Type type) {
  ASSERT(renderer->window->backend.type != WindowManagerBackend::Type::kLast);

  if (type == RendererBackend::Type::kLast) {
    LOG(ERROR) << "Unset RendererBackend.";
    return false;
  }


  renderer->backend = GetRendererBackend(type);
  renderer->backend.renderer = renderer;  // Set the back pointer.

  // Initialize the backend.
  auto& interface = renderer->backend.interface;
  return interface.Init(&renderer->backend);
}

// An null backend renderer can happen if Shutdown was called before the
// destructor.
void ShutdownRenderer(Renderer* renderer) {
  ASSERT(renderer->valid());
  auto& interface = renderer->interface();
  interface.Shutdown(&renderer->backend);
}

// void WindowSizeChanged(Renderer* renderer, uint32_t width, uint32_t height) {}

bool DrawFrame(Renderer* renderer, Camera* camera) {
  ASSERT(renderer->valid());
  auto& interface = renderer->interface();
  return interface.DrawFrame(&renderer->backend, camera);
}

}  // namespace warhol
