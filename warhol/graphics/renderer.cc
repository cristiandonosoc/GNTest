// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/renderer.h"

#include "warhol/utils/assert.h"

namespace warhol {

Renderer::Renderer() = default;
Renderer::~Renderer() {
  ShutdownRenderer(this);
}

// void WindowSizeChanged(Renderer* renderer, uint32_t width, uint32_t height) {}

bool DrawFrame(Renderer* renderer, SDLContext* sdl_context, Camera* camera) {
  switch (renderer->backend_type) {
    case Renderer::BackendType::kVulkan:
      return DrawFrameVulkan(renderer->vulkan_renderer.get(), sdl_context,
                             camera);
    case Renderer::BackendType::kLast:
      break;
  }

  NOT_REACHED("Invalid renderer backend.");
  return false;
}

bool InitRendererWithVulkanAndSDL(Renderer* renderer, SDLContext* sdl_context) {
  renderer->backend_type = Renderer::BackendType::kVulkan;
  renderer->window_manager = Renderer::WindowManager::kSDL;

  renderer->vulkan_renderer = std::make_unique<RendererBackendVulkan>();
  return InitVulkanRendererBackendWithSDL(renderer->vulkan_renderer.get(),
                                          sdl_context);
}

// An null backend renderer can happen if Shutdown was called before the
// destructor.
bool ShutdownRenderer(Renderer* renderer) {
  switch (renderer->backend_type) {
    case Renderer::BackendType::kVulkan:
      if (renderer->vulkan_renderer)
        return ShutdownVulkanBackend(renderer, renderer->vulkan_renderer.get());
      break;
    case Renderer::BackendType::kLast:
      NOT_REACHED("Unknown renderer backend.");
      break;
  }

  return true;
}

const char* Renderer::BackendTypeToString(Renderer::BackendType bt) {
  switch (bt) {
    case Renderer::BackendType::kVulkan: return "Vulkan";
    case Renderer::BackendType::kLast: return "Last";
  }

  NOT_REACHED("Unknown Backend Type.");
  return nullptr;
}

const char* Renderer::WindowManagerToString(Renderer::WindowManager wm) {
  switch (wm) {
    case Renderer::WindowManager::kSDL: return "SDL";
    case Renderer::WindowManager::kLast: return "Last";
  }

  NOT_REACHED("Unknown Window Manager.");
  return nullptr;
}

}  // namespace warhol
