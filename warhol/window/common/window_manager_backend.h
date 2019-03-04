// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>
#include <utility>

#include "warhol/utils/macros.h"

namespace warhol {

struct InputState;
struct WindowManager;   // Defined later in file.

struct WindowEvent {
  enum class Type {
    kQuit,
    kWindowResize,
    kLast,
  };
  static const char* TypeToString(Type);

  Type type = Type::kLast;
};

struct WindowManagerBackend {
  enum class Type {
    kSDLVulkan,
    kLast,    // DO NOT specialize with numbers.
  };
  static const char* TypeToString(Type);

  WindowManagerBackend();
  WindowManagerBackend(Type);
  virtual ~WindowManagerBackend();

  bool valid() const { return window_manager != nullptr; }

  Type type = Type::kLast;
  WindowManager* window_manager = nullptr;  // Not owning.

  // Interface -----------------------------------------------------------------

  // Must leave the backend in a |valid()| state.
  virtual void Init(WindowManager*, uint64_t flags) = 0;

  // Must leave the backend in an |!valid()| state.
  virtual void Shutdown() = 0;

  // Can only be called in a |valid()| state.
  virtual std::pair<WindowEvent*, size_t> NewFrame(InputState*) = 0;

  // *** VULKAN SPECIFIC ***
  // These functions must be subclassed. Calling them in a backend that doesn't
  // support them will assert a failure (see window_manager_backend.cc).

  virtual std::vector<const char*> GetVulkanInstanceExtensions();

  // |vk_instance| & |surface_khr| must be casted to the right type in the
  // implementation. This is so that we don't need to forward declare vulkan
  // typedefs.
  virtual bool CreateVulkanSurface(void* vk_instance, void* surface_khr);
};

// Backend Suscription ---------------------------------------------------------

// Each backend, upon application startup, must suscribe a function that will
// be called to create a that particular WindowManagerBackend.
using WindowManagerBackendFactory = std::unique_ptr<WindowManagerBackend> (*)();
void SuscribeWindowManagerBackendFactory(WindowManagerBackend::Type,
                                         WindowManagerBackendFactory);

std::unique_ptr<WindowManagerBackend>
CreateWindowManagerBackend(WindowManagerBackend::Type);

}  // namespace warhol
