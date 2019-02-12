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
  bool valid() const { return window_manager != nullptr && data != nullptr; }

  // DO NOT add numbers to this enum.
  enum class Type {
    kSDLVulkan,
    kLast,    // DO NOT specialize with numbers.
  };
  static const char* TypeToString(Type);

  WindowManagerBackend();
  ~WindowManagerBackend();
  DELETE_COPY_AND_ASSIGN(WindowManagerBackend);
  DECLARE_MOVE_AND_ASSIGN(WindowManagerBackend);

  // IMPORTANT: If you add a function here, remember to handle it (specially
  //            constructors) in the .cc file.
  struct Interface {
    bool (*Init)(WindowManagerBackend*, uint64_t flags) = nullptr;
    std::pair<WindowEvent*, size_t> (*NewFrame)(WindowManagerBackend*,
                                                InputState*) = nullptr;
    void (*Shutdown)(WindowManagerBackend*) = nullptr;

    // *** VULKAN SPECIFIC ***
    std::vector<const char*>
    (*GetVulkanInstanceExtensions)(WindowManagerBackend*) = nullptr;

    // |vk_instance| & |surface_khr| must be casted to the right type in the
    // implementation.
    // This is so that we don't need to typedef the values and we don't create
    // unnecessary dependencies on the graphics libraries.
    bool (*CreateVulkanSurface)(WindowManagerBackend*, void* vk_instance,
                                void* surface_khr);
  };

  Type type = Type::kLast;
  Interface interface = {};

  WindowManager* window_manager = nullptr;
  void* data = nullptr;  // Underlying memory of backend.
};

void SetWindowManagerBackendInterfaceTemplate(
    WindowManagerBackend::Type, WindowManagerBackend::Interface interface);

// Creates an un-initialized backend for |type|.
// That type MUST have been set for that type.
// Initialize must be called before using this.
WindowManagerBackend GetWindowManagerBackend(WindowManagerBackend::Type type);

}  // namespace warhol
