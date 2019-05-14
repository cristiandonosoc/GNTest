// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/math/math.h"
#include "warhol/utils/glm_impl.h"

namespace warhol {

struct RenderCommand;
struct WindowManager;

namespace vulkan {

struct UBO {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

struct VulkanRendererBackend;

// Collection of functions that implement the low-level functionality exposed
// by the vulkan renderer backend.
// This is mainly an ordering separation, as this all could be implemented in
// renderer_backend.cc as non-exported functions.

void VulkanBackendInitImpl(VulkanRendererBackend*, WindowManager*);

// TODO(Cristian): Review if this should be here.
//                 The RendererBackendInterface should have a display changed
//                 call.
void VulkanBackendRecreateSwapChain(VulkanRendererBackend* vulkan,
                                    Pair<uint32_t> screen_size);

// Execute Commands Functions --------------------------------------------------

void VulkanBackendStartFrame(VulkanRendererBackend*);
void VulkanBackendDrawMesh(VulkanRendererBackend*, RenderCommand*);
void VulkanBackendEndFrame(VulkanRendererBackend*);

}  // namespace vulkan
}  // namespace warhol
