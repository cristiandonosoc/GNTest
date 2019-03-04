// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/window_manager.h"

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {

// WindowManager ---------------------------------------------------------------

WindowManager::WindowManager() = default;
WindowManager::~WindowManager() {
  if (valid())
    WindowManagerShutdown(this);
}

void WindowManagerInit(WindowManager* window, WindowManagerBackend::Type type,
                       uint64_t flags) {
  ASSERT(type != WindowManagerBackend::Type::kLast);

  window->backend = CreateWindowManagerBackend(type);
  window->backend->Init(window, flags);
}

void WindowManagerShutdown(WindowManager* window) {
  ASSERT(window->valid());
  window->backend->Shutdown();
}

std::pair<WindowEvent*, size_t>
WindowManagerNewFrame(WindowManager* window, InputState* input) {
  ASSERT(window->valid());
  return window->backend->NewFrame(input);
}

std::vector<const char*>
WindowManagerGetVulkanInstanceExtensions(WindowManager* window) {
  ASSERT(window->valid());
  return window->backend->GetVulkanInstanceExtensions();
}

// |vk_instance| & |surface_khr| must be casted to the right type in the
// implementation. This is so that we don't need to forward declare vulkan
// typedefs.
bool WindowManagerCreateVulkanSurface(WindowManager* window, void* vk_instance,
                                      void* surface_khr) {
  ASSERT(window->valid());
  return window->backend->CreateVulkanSurface(vk_instance, surface_khr);
}

}  // namespace warhol
