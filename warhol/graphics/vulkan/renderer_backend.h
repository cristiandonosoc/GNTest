// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>

#include "warhol/utils/macros.h"

namespace warhol {

class SDLContext;
struct RendererBackend;
struct Camera;
struct Renderer;

namespace vulkan {

struct Context;

struct VulkanRendererBackend {
  VulkanRendererBackend();
  ~VulkanRendererBackend();
  DELETE_COPY_AND_ASSIGN(VulkanRendererBackend);
  DELETE_MOVE_AND_ASSIGN(VulkanRendererBackend);

  std::unique_ptr<vulkan::Context> context;
};

bool InitRendererBackend(RendererBackend*);
bool ExecuteCommands(RendererBackend*);
bool ShutdownRendererBackend(RendererBackend*);
bool DrawFrame(RendererBackend*, Camera*);

}  // namespace vulkan
}  // namespace warhol
