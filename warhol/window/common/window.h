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

  int width = 0;
  int height = 0;

  static constexpr size_t kMaxUtf8Chars = 255;
  char utf8_chars_inputted[kMaxUtf8Chars + 1];
  int utf8_index = 0;

  WindowBackendType backend_type = WindowBackendType::kLast;
  std::unique_ptr<WindowBackend> backend;
};

// Window API -----------------------------------------------------------

inline bool Valid(Window* wm) { return !!wm->backend; }

// TODO(Cristian): Pass in WindowInitOptions!
struct InitWindowConfig {
  bool borderless = false;
  bool fullscreen = false;
  bool hidden = false;
  bool resizable = false;

  // Mutual exclusive, maximized wins.
  bool minimized = false;
  bool maximized = false;
};
bool InitWindow(Window*, WindowBackendType, InitWindowConfig*);

// Will be called on destructor if window manager is valid.
void ShutdownWindow(Window*);

// Gets the window events and calculate inputs.
// TODO(donosoc): Eventually separate window from inputs, even if behind the
//                scenes they could be using the same backend (eg. SDL).
List<WindowEvent> UpdateWindow(Window*, InputState*);

// Some window managers have an explicit call for swapping buffers, notably
// OpenGL. Since each window manager is different, we let the backend take
// care of that functionality.
//
// In renderers that don't need this, this will be a no-op.
//
// NOTE: If v-sync is enabled, this will block on it.
void WindowSwapBuffers(Window*);

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
