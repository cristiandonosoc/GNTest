// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <utility>
#include <vector>

#include "warhol/utils/macros.h"

namespace warhol {

struct InputState;
struct WindowManagerBackend;

enum class WindowEvent {
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
// be called to create a that particular WindowManagerBackend.
using WindowBackendFactoryFunction =
    std::unique_ptr<WindowManagerBackend> (*)();
void SuscribeWindowBackendFactoryFunction(WindowBackendType,
                                          WindowBackendFactoryFunction);

struct WindowManager {
  ~WindowManager();   // "RAII" semantics.

  size_t width = 0;
  size_t height = 0;
  float frame_delta = 0;          // Delta within the last frame in seconds.
  float frame_delta_average = 0;  // Rolling average over kRollingAverageFrames.
  float frame_rate = 0;           // 1 / frame_delta_average.
  float seconds = 0;              // Seconds since Init was called.

  WindowBackendType backend_type = WindowBackendType::kLast;
  std::unique_ptr<WindowManagerBackend> backend;
};

// WindowManager API -----------------------------------------------------------

inline bool Valid(WindowManager* wm) { return !!wm->backend; }

// If false, the window manager will be invalid.
// TODO(Cristian): Pass in flags.
bool InitWindowManager(WindowManager*, WindowBackendType);

// Will be called on destructor if window manager is valid.
void ShutdownWindowManager(WindowManager*);

std::pair<WindowEvent*, size_t>
UpdateWindowManager(WindowManager*, InputState*);

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
bool WindowManagerCreateVulkanSurface(WindowManager*,
                                      void* vk_instance,
                                      void* surface_khr);

// WIndowManagerBackend --------------------------------------------------------

struct WindowManagerBackend {
  virtual ~WindowManagerBackend();

  WindowManager* window_manager = nullptr;  // Not owning.

  // Interface -----------------------------------------------------------------

  virtual bool Init(WindowManager*) = 0;
  virtual void Shutdown() = 0;
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

}  // namespace warhol
