// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/memory_utils.h"

#include "warhol/graphics/common/image.h"
#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/commands.h"
#include "warhol/graphics/vulkan/image_utils.h"
#include "warhol/graphics/vulkan/utils.h"

namespace warhol {
namespace vulkan {

// VkBuffer --------------------------------------------------------------------

MemoryBacked<VkBuffer>
AllocBuffer(Context* context, AllocBufferConfig* config) {
  ASSERT(config->memory_usage != MemoryUsage::kNone);

  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = config->size;
  buffer_info.usage = config->usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkBuffer buffer;
  if (!VK_CALL(vkCreateBuffer, *context->device, &buffer_info, nullptr,
                               &buffer)) {
    return {};
  }
  Handle<VkBuffer> buffer_handle(context, buffer);

  VkMemoryRequirements memory_reqs;
  vkGetBufferMemoryRequirements(*context->device, buffer, &memory_reqs);

  AllocateConfig alloc_config = {};
  alloc_config.size = memory_reqs.size;
  alloc_config.align = memory_reqs.alignment;
  alloc_config.memory_type_bits = memory_reqs.memoryTypeBits;
  alloc_config.memory_usage = config->memory_usage;
  alloc_config.alloc_type = AllocationType::kBuffer;
  Allocation allocation = {};
  if (!Allocate(context, &context->allocator, alloc_config, &allocation))
    return {};

  if (!VK_CALL(vkBindBufferMemory, *context->device, buffer, *allocation.memory,
               allocation.offset)) {
    return {};
  }

  MemoryBacked<VkBuffer> result = {};
  result.handle = std::move(buffer_handle);
  result.allocation = std::move(allocation);

  return result;
}

bool CopyBuffer(Context* context, VkBuffer src_buffer, VkBuffer dst_buffer,
                VkDeviceSize size) {
  Handle<VkCommandBuffer> command_buffer = BeginSingleTimeCommands(context);
  if (!command_buffer.has_value())
    return false;

  VkBufferCopy buffer_copy = {};
  buffer_copy.size = size;
  buffer_copy.srcOffset = 0;
  buffer_copy.dstOffset = 0;
  vkCmdCopyBuffer(*command_buffer, src_buffer, dst_buffer, 1, &buffer_copy);

  if (!EndSingleTimeCommands(context, *command_buffer))
    return false;
  return true;
}

}  // namespace vulkan
}  // namespace warhol
