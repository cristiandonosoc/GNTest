// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/renderer.h"

#include "warhol/utils/assert.h"
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
  ShutdownRenderer(this);
}

namespace {

bool InitVulkan(Renderer* renderer) {
#ifndef WARHOL_VULKAN_ENABLED
  NOT_REACHED("Vulkan support not compiled in.");
#else
  return vulkan::InitRendererBackend(&renderer->backend);
#endif
}

}  // namespace

bool InitRenderer(Renderer* renderer) {
  ASSERT(renderer->window->backend.type != WindowManagerBackend::Type::kLast);

  // Set the back pointer.
  renderer->backend.renderer = renderer;

  switch (renderer->backend_type) {
    case Renderer::BackendType::kVulkan:
      return InitVulkan(renderer);
    case Renderer::BackendType::kLast:
      break;
  }

  NOT_REACHED("Unknown backend type.");
  return false;
}

// An null backend renderer can happen if Shutdown was called before the
// destructor.
bool ShutdownRenderer(Renderer* renderer) {
  RendererBackend* backend = &renderer->backend;
  switch (renderer->backend_type) {
    case Renderer::BackendType::kVulkan:
      if (backend->valid())
        return backend->interface.ShutdownFunction(backend);
      break;
    case Renderer::BackendType::kLast:
      NOT_REACHED("Unknown renderer backend.");
      break;
  }
  Clear(&renderer->backend);
  return true;
}

// void WindowSizeChanged(Renderer* renderer, uint32_t width, uint32_t height) {}

bool DrawFrame(Renderer* renderer, Camera* camera) {
  RendererBackend* backend = &renderer->backend;
  ASSERT(backend->valid());
  switch (renderer->backend_type) {
    case Renderer::BackendType::kVulkan:
      return backend->interface.DrawFrameFunction(backend, camera);
    case Renderer::BackendType::kLast:
      break;
  }

  NOT_REACHED("Invalid renderer backend.");
  return false;
}

// Utils -----------------------------------------------------------------------

const char* Renderer::BackendTypeToString(Renderer::BackendType bt) {
  switch (bt) {
    case Renderer::BackendType::kVulkan: return "Vulkan";
    case Renderer::BackendType::kLast: return "Last";
  }

  NOT_REACHED("Unknown Backend Type.");
  return nullptr;
}

}  // namespace warhol
