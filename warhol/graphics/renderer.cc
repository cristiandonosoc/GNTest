// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/renderer.h"

#include "warhol/utils/assert.h"

#ifdef WARHOL_VULKAN_ENABLED
#include "warhol/graphics/renderer_backend_vulkan.h"
#endif

namespace warhol {

// BackendInterface ------------------------------------------------------------

BackendInterface::BackendInterface() = default;
BackendInterface::~BackendInterface() {
  if (valid()) {
    ASSERT(data);
    ShutdownFunction(this);   // Frees data.
  }
  Clear(this);
}

BackendInterface::BackendInterface(BackendInterface&& other)
    : renderer(other.renderer),
      InitFunction(other.InitFunction),
      ShutdownFunction(other.ShutdownFunction),
      DrawFrameFunction(other.DrawFrameFunction),
      data(other.data) {
  Clear(&other);
}

BackendInterface& BackendInterface::operator=(BackendInterface&& other) {
  renderer = other.renderer;
  InitFunction = other.InitFunction;
  ShutdownFunction = other.ShutdownFunction;
  DrawFrameFunction = other.DrawFrameFunction;
  data = other.data;
  Clear(&other);
  return *this;
}

void Clear(BackendInterface* bi) {
  bi->renderer = nullptr;
  bi->InitFunction = nullptr;
  bi->ShutdownFunction = nullptr;
  bi->DrawFrameFunction = nullptr;
  bi->data = nullptr;
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
  return InitVulkanRenderer(&renderer->backend_interface);
#endif
}

}  // namespace

bool InitRenderer(Renderer* renderer) {
  ASSERT(renderer->window_manager != Renderer::WindowManager::kLast);
  ASSERT(renderer->sdl_context != nullptr);

  // Set the back pointer.
  renderer->backend_interface.renderer = renderer;

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
  BackendInterface* bi = &renderer->backend_interface;
  switch (renderer->backend_type) {
    case Renderer::BackendType::kVulkan:
      if (bi->valid())
        return bi->ShutdownFunction(bi);
      break;
    case Renderer::BackendType::kLast:
      NOT_REACHED("Unknown renderer backend.");
      break;
  }
  Clear(&renderer->backend_interface);
  return true;
}

// void WindowSizeChanged(Renderer* renderer, uint32_t width, uint32_t height) {}

bool DrawFrame(Renderer* renderer, Camera* camera) {
  BackendInterface* bi = &renderer->backend_interface;
  ASSERT(bi->valid());
  switch (renderer->backend_type) {
    case Renderer::BackendType::kVulkan:
      return bi->DrawFrameFunction(bi, camera);
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

const char* Renderer::WindowManagerToString(Renderer::WindowManager wm) {
  switch (wm) {
    case Renderer::WindowManager::kSDL: return "SDL";
    case Renderer::WindowManager::kLast: return "Last";
  }

  NOT_REACHED("Unknown Window Manager.");
  return nullptr;
}

}  // namespace warhol
