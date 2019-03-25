// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <memory>
#include <vector>

#include "warhol/utils/macros.h"
#include "warhol/window/common/window_backend.h"

namespace warhol {

struct InputState;
struct WindowBackend;

enum class WindowEvent : uint32_t {
  kQuit,
  kWindowResize,
  kLast,
};

enum class WindowBackendType {
  kSDLOpenGL,
  kSDLVulkan,
  kLast,
};
const char* ToString(WindowBackendType);

// Each backend, upon application startup, must suscribe a function that will
// be called to create a that particular WindowBackend.
using WindowBackendFactoryFunction =
    std::unique_ptr<WindowBackend> (*)();
void SuscribeWindowBackendFactoryFunction(WindowBackendType,
                                          WindowBackendFactoryFunction);

struct Window {
  ~Window();   // "RAII" semantics.

  size_t width = 0;
  size_t height = 0;
  float frame_delta = 0;          // Delta within the last frame in seconds.
  float frame_delta_average = 0;  // Rolling average over kRollingAverageFrames.
  float frame_rate = 0;           // 1 / frame_delta_average.
  float seconds = 0;              // Seconds since Init was called.

  WindowBackendType backend_type = WindowBackendType::kLast;
  std::unique_ptr<WindowBackend> backend;
};

// Window API -----------------------------------------------------------

inline bool Valid(Window* wm) { return !!wm->backend; }

// If false, the window manager will be invalid.
// TODO(Cristian): Pass in flags.
bool InitWindow(Window*, WindowBackendType);

// Will be called on destructor if window manager is valid.
void ShutdownWindow(Window*);

std::pair<WindowEvent*, size_t> UpdateWindow(Window*, InputState*);

// *** VULKAN SPECIFIC ***
//
// Call it only on Window that have a backend that support these vulkan
// functions. See window/common/window_manager_backend.h for more details.

// Gets the extension that the window manager needs to work with vulkan.
std::vector<const char*> WindowGetVulkanInstanceExtensions(Window*);

// |vk_instance| & |surface_khr| must be casted to the right type in the
// implementation. This is so that we don't need to forward declare vulkan
// typedefs.
bool WindowCreateVulkanSurface(Window*, void* vk_instance, void* surface_khr);

}  // namespace warhol
