// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/staging_manager.h"

#include "warhol/graphics/vulkan/commands.h"
#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/utils.h"
#include "warhol/utils/macros.h"

namespace warhol {
namespace vulkan {

namespace {

bool InitStagingBuffer(Context* context, StagingManager* manager,
                       StagingBuffer* out) {
    // Memory back the buffer.
    AllocBufferConfig alloc_config = {};
    alloc_config.size = manager->buffer_size;
    alloc_config.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    alloc_config.memory_usage = MemoryUsage::kCPUToGPU;
    auto buffer = AllocBuffer(context, &alloc_config);
    if (!buffer.has_value())
      return false;

    // Create the command buffer.
    auto command_buffer = CreateCommandBuffer(context, *manager->command_pool);
    if (!command_buffer.has_value())
      return false;

    // Add a begin pass to the command buffer.
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (!VK_CALL(vkBeginCommandBuffer, *command_buffer, &begin_info))
      return false;

    // Create the fence.
    VkFence fence_out;
    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (!VK_CALL(vkCreateFence, *context->device, &fence_create_info, nullptr,
                 &fence_out)) {
      return false;
    }
    Handle<VkFence> fence(context, fence_out);

    out->buffer = std::move(buffer);
    out->command_buffer = std::move(command_buffer);
    out->fence = std::move(fence);

    return true;
}
}  // namespace

bool Init(Context* context, StagingManager* manager) {
  // This command pool is to be used over and over.
  VkCommandPool command_pool_out;
  VkCommandPoolCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  create_info.queueFamilyIndex =
      context->physical_device_info.queue_family_indices.graphics;
  if (!VK_CALL(vkCreateCommandPool, *context->device, &create_info, nullptr,
               &command_pool_out)) {
    return false;
  }
  manager->command_pool.Set(context, command_pool_out);

  // Allocate the buffer and associate the values.
  for (int i = 0; ARRAY_SIZE(manager->buffers); i++) {
    StagingBuffer staging_buffer = {};
    if (!InitStagingBuffer(context, manager, &staging_buffer))
      return false;
    manager->buffers[i] = std::move(staging_buffer);
  }
}

}  // namespace vulkan
}  // namespace warhol
