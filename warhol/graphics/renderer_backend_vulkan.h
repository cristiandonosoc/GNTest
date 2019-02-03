// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>

#include "warhol/utils/macros.h"

namespace warhol {

class SDLContext;

struct Camera;
struct Renderer;

namespace vulkan {
struct Context;
}  // namespace vulkan

struct RendererBackendVulkan {
  RendererBackendVulkan();
  ~RendererBackendVulkan();
  DELETE_COPY_AND_ASSIGN(RendererBackendVulkan);
  DELETE_MOVE_AND_ASSIGN(RendererBackendVulkan);

  std::unique_ptr<vulkan::Context> context;
};

bool InitVulkanRendererBackendWithSDL(RendererBackendVulkan*, SDLContext*);
bool ShutdownVulkanBackend(Renderer*, RendererBackendVulkan*);

bool DrawFrameVulkan(RendererBackendVulkan*, SDLContext*, Camera*);

}  // namespace warhol
