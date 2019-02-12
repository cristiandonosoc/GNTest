// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/window_manager.h"

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {

// WindowManagerBackend --------------------------------------------------------

namespace {

void Clear(WindowManagerBackend* backend) {
  backend->window_manager = nullptr;
  backend->data = nullptr;

  backend->interface.Init = nullptr;
  backend->interface.NewFrame = nullptr;
  backend->interface.Shutdown = nullptr;

  backend ->interface.GetVulkanInstanceExtensions = nullptr;
  backend->interface.CreateVulkanSurface = nullptr;
}

void Move(WindowManagerBackend* from, WindowManagerBackend* to) {
  to->window_manager = from->window_manager;
  to->data = from->data;

  to->interface.Init = from->interface.Init;
  to->interface.NewFrame = from->interface.NewFrame;
  to->interface.Shutdown = from->interface.Shutdown;

  // Vulkan.
  to->interface.GetVulkanInstanceExtensions =
      from->interface.GetVulkanInstanceExtensions;
  to->interface.CreateVulkanSurface = from->interface.CreateVulkanSurface;

  Clear(to);
}

}  // namespace

WindowManagerBackend::WindowManagerBackend() = default;
WindowManagerBackend::~WindowManagerBackend() {
  if (valid()) {
    ASSERT(data);
    interface.Shutdown(this);
  }
  Clear(this);
}

WindowManagerBackend::WindowManagerBackend(WindowManagerBackend&& other) {
  Move(&other, this);
}

WindowManagerBackend&
WindowManagerBackend::operator=(WindowManagerBackend&& other) {
  if (this != &other) {
    Move(&other, this);
  }
  return *this;
}

const char*
WindowManagerBackend::TypeToString(WindowManagerBackend::Type type) {
  switch (type) {
    case WindowManagerBackend::Type::kSDLVulkan: return "SDLVulkan";
    case WindowManagerBackend::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown WindowManagerBackend::Type.");
  return nullptr;
}

// WindowManager ---------------------------------------------------------------

bool InitWindowManager(WindowManager* window,
                       WindowManagerBackend::Type type,
                       uint64_t flags) {
  if (type == WindowManagerBackend::Type::kLast) {
    LOG(ERROR) << "Unset WindowManager type.";
    return false;
  }

  window->backend = GetWindowManagerBackend(type);
  window->backend.window_manager = window;  // Set the backpointer.

  auto& interface = window->backend.interface;
  return interface.Init(&window->backend, flags);
}



}  // namespace warhol
