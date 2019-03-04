// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <utility>
#include <vector>

#include "warhol/utils/macros.h"
#include "warhol/window/common/window_manager_backend.h"

namespace warhol {

struct WindowManagerBackend;

struct WindowManager {
  static constexpr size_t kRollingAverageFrames = 60;

  WindowManager();
  ~WindowManager();
  DELETE_COPY_AND_ASSIGN(WindowManager);
  DELETE_MOVE_AND_ASSIGN(WindowManager);

  bool valid() const { return backend && backend->valid(); }

  size_t width = 0;
  size_t height = 0;
  float frame_delta = 0;          // Delta within the last frame in seconds.
  float frame_delta_average = 0;  // Rolling average over kRollingAverageFrames.
  float frame_rate = 0;           // 1 / frame_delta_average.
  float seconds = 0;              // Seconds since Init was called.

  std::unique_ptr<WindowManagerBackend> backend;
};

// WindowManager API -----------------------------------------------------------

// WindowManager must be already set with |type|.
void WindowManagerInit(WindowManager*, WindowManagerBackend::Type,
                       uint64_t flags);

void WindowManagerShutdown(WindowManager*);

std::pair<WindowEvent*, size_t>
WindowManagerNewFrame(WindowManager*, InputState*);

// *** VULKAN SPECIFIC ***
//
// Call it only on WindowManager that have a backend that support these vulkan
// functions. See window/common/window_manager_backend.h for more details.

// Gets the extension that the window manager needs to work with vulkan.
std::vector<const char*>
WindowManagerGetVulkanInstanceExtensions(WindowManager*);

// |vk_instance| & |surface_khr| must be casted to the right type in the
// implementation. This is so that we don't need to forward declare vulkan
// typedefs.
bool WindowManagerCreateVulkanSurface(WindowManager*, void* vk_instance,
                                      void* surface_khr);

}  // namespace warhol
