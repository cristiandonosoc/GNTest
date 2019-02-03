// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <memory>

#include "warhol/graphics/renderer_backend_vulkan.h"
#include "warhol/utils/macros.h"

namespace warhol {

class SDLContext;

struct Camera;

struct Renderer {
  enum class BackendType : uint32_t {
    kVulkan,
    kLast,
  };
  const char* BackendTypeToString(BackendType);

  enum class WindowManager : uint32_t {
    kSDL,
    kLast,
  };
  const char* WindowManagerToString(WindowManager);

  Renderer();
  ~Renderer();
  DELETE_COPY_AND_ASSIGN(Renderer);
  DELETE_MOVE_AND_ASSIGN(Renderer);

  BackendType backend_type = BackendType::kLast;
  WindowManager window_manager = WindowManager::kLast;

  std::unique_ptr<RendererBackendVulkan> vulkan_renderer;
};

bool InitRendererWithVulkanAndSDL(Renderer*, SDLContext*);
bool ShutdownRenderer(Renderer*);

void WindowSizeChanged(Renderer*, uint32_t width, uint32_t height);
bool DrawFrame(Renderer*, SDLContext* sdl_context, Camera*);


}  // namespace
