// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <utility>
#include <vector>

namespace warhol {

enum class WindowEvent : uint32_t;

struct InputState;
struct Window;

struct WindowBackend {
  virtual ~WindowBackend();

  // Interface -----------------------------------------------------------------

  virtual bool Init(Window*) = 0;
  virtual void Shutdown() = 0;
  virtual std::pair<WindowEvent*, size_t> NewFrame(Window*, InputState*) = 0;

  // *** VULKAN SPECIFIC ***

  // These functions must be subclassed if needed. If a backend doesn't need
  // them, they can choose not to do so.
  // Calling them in a backend that doesn't support them will assert a failure.
  // (see window_backend.cc).

  // Instance extensions required by this window manager.
  virtual std::vector<const char*> GetVulkanInstanceExtensions();

  // |vk_instance| & |surface_khr| must be casted to the right type in the
  // implementation. This is so that we don't need to forward declare vulkan
  // typedefs.
  virtual bool CreateVulkanSurface(void* vk_instance, void* surface_khr);
};

}  // namespace warhol
