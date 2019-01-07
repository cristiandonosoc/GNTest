// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/commands.h"

#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/utils.h"

namespace warhol {
namespace vulkan {

Handle<VkCommandBuffer> BeginSingleTimeCommands(Context* context) {
  // Allocate a temporary command buffer for these copy operations.
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;
  // TODO(Cristian): Have a separate command pool for these transient commands.
  alloc_info.commandPool = *context->command_pool;

  VkCommandBuffer command_buffer;
  if (!VK_CALL(vkAllocateCommandBuffers, *context->device, &alloc_info,
               &command_buffer)) {
    return {};
  }
  // In order to ensure freeing.
  Handle<VkCommandBuffer> command_buffer_handle(context, command_buffer);

  // This command buffer will be used only once.
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  if (!VK_CALL(vkBeginCommandBuffer, command_buffer, &begin_info))
    return {};
  return command_buffer_handle;
}

bool EndSingleTimeCommands(Context* context, VkCommandBuffer command_buffer) {
  if (!VK_CALL(vkEndCommandBuffer, command_buffer))
    return false;

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;

  // We submit and wait for the queue to be idle.
  if (!VK_CALL(vkQueueSubmit, context->graphics_queue, 1, &submit_info,
               nullptr) ||
      !VK_CALL(vkQueueWaitIdle, context->graphics_queue)) {
    return false;
  }

  return true;
}

}  // namespace vulkan
}  // namespace warhol
