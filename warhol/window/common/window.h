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
  // Amount of frames to keep track of in order to get an average frame time.
  static constexpr int kFrameTimesCounts = 128;

  ~Window();   // "RAII" semantics.

  int width = 0;
  int height = 0;

  // TODO(Cristian): All timing shouldn't be within window, but on platform!

  // Total time since the start of the game.
  uint64_t total_time = 0;
  float seconds = 0;
  float frame_delta = 0;

  float frame_delta_accum = 0;  // The accumulated average.
  float frame_delta_average = 0;
  float frame_rate = 0;

  // TODO(Cristian): When we're interested, start tracking these times.
  float frame_times[kFrameTimesCounts];
  int frame_times_index = 0;

  WindowBackendType backend_type = WindowBackendType::kLast;
  std::unique_ptr<WindowBackend> backend;
};

// Window API -----------------------------------------------------------

inline bool Valid(Window* wm) { return !!wm->backend; }

// TODO(Cristian): Pass in WindowInitOptions!
bool InitWindow(Window*, WindowBackendType);

// Will be called on destructor if window manager is valid.
void ShutdownWindow(Window*);

LinkedList<WindowEvent> UpdateWindow(Window*, InputState*);

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
