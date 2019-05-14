// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/vulkan/def.h"

#include "warhol/graphics/vulkan/handle.h"

namespace warhol {
namespace vulkan {

struct Context;

// These are helpers about common functionality involving command buffers and
// commands.

Handle<VkCommandBuffer> CreateCommandBuffer(
    Context*,
    VkCommandPool,
    VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

// Will createa a one-off command buffer and call vkBeginCommandBuffer on it.
Handle<VkCommandBuffer> BeginSingleTimeCommands(Context*);

// Ends a command buffer and submits it.
// Will call vkQueueWaitIdle on it, so this call blocks.
bool EndSingleTimeCommands(Context*, VkCommandBuffer);

}  // namespace vulkan
}  // namespace warhol
