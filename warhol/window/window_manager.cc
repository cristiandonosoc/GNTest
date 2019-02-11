// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/window/window_manager.h"

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

#ifdef WARHOL_SDL_ENABLED
#include "warhol/window/sdl/window_manager.h"
#endif

namespace warhol {

const char* WindowEvent::TypeToString(WindowEvent::Type event) {
  switch (event) {
    case WindowEvent::Type::kQuit: return "Quit";
    case WindowEvent::Type::kWindowResize: return "WindowResize";
    case WindowEvent::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown WindowEvent.");
  return nullptr;
}

// WindowManagerBackend --------------------------------------------------------

namespace {

void Clear(WindowManagerBackend* backend) {
  backend->NewFrameFunction = nullptr;
  backend->ShutdownFunction = nullptr;
  backend->window_manager = nullptr;
  backend->data = nullptr;
}

void Move(WindowManagerBackend* from, WindowManagerBackend* to) {
  to->NewFrameFunction = from->NewFrameFunction;
  to->ShutdownFunction = from->ShutdownFunction;
  to->window_manager = from->window_manager;
  to->data = from->data;
  Clear(to);
}

}  // namespace

WindowManagerBackend::WindowManagerBackend() = default;
WindowManagerBackend::~WindowManagerBackend() {
  if (valid()) {
    ASSERT(data);
    ShutdownFunction(this);
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

// WindowManager ---------------------------------------------------------------

namespace {

bool InitSDLVulkan(WindowManagerBackend* backend, uint64_t flags) {
#ifndef WARHOL_SDL_ENABLED
#error SDL support is not compiled in.
#else
  return sdl::InitSDLVulkan(backend, flags);
#endif
}

}  // namespace

bool InitWindowManager(WindowManager* window, uint64_t flags) {
  if (window->type == WindowManager::Type::kLast) {
    LOG(ERROR) << "Unset WindowManager type.";
    return false;
  }

  // Set the backpointer.
  window->backend.window_manager = window;

  switch (window->type) {
    case WindowManager::Type::kSDLVulkan:
      return InitSDLVulkan(&window->backend, flags);
    case WindowManager::Type::kLast:
      break;
  }

  NOT_REACHED("Unknown WindowManager::Type.");
  return false;
}


const char* WindowManager::TypeToString(WindowManager::Type type) {
  switch (type) {
    case WindowManager::Type::kSDLVulkan: return "SDLVulkan";
    case WindowManager::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown WindowManager::Type.");
  return nullptr;
}

std::vector<const char*> GetVulkanInstanceExtensions(WindowManager* window) {
#ifndef WARHOL_SDL_ENABLED
  NOT_REACHED("No surface backend enabled.");
#else
  return sdl::GetVulkanInstanceExtensions(&window->backend);
#endif
}

// Will be casted to the right type in the .cc
// This is so that we don't need to typedef the values and we don't create
// unnecessary dependencies on the graphics libraries.
bool CreateVulkanSurface(WindowManager* window, void* vk_instance,
                         void* surface_khr) {
#ifndef WARHOL_SDL_ENABLED
  NOT_REACHED("No surface backend enabled.");
#else
  return sdl::CreateVulkanSurface(&window->backend, vk_instance, surface_khr);
#endif
}

}  // namespace warhol
