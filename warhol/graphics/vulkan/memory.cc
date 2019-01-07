// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/memory.h"

#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/image_utils.h"
#include "warhol/graphics/vulkan/utils.h"
#include "warhol/utils/scope_trigger.h"

namespace warhol {
namespace vulkan {

namespace {

// |type_filter| comes from the |memoryTypeBits| field in VkMemoryRequirements.
// That field establishes the index of all the valid memory types within the
// |memoryTypes| array in VkPhysicalDeviceMemoryProperties.
bool FindMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& properties,
                         uint32_t type_filter,
                         VkMemoryPropertyFlags desired_flags,
                         uint32_t* memory_type_index) {
  for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
    const VkMemoryType& memory_type = properties.memoryTypes[i];
    if ((type_filter & (1 << i) &&
        (memory_type.propertyFlags & desired_flags) == desired_flags)) {
      *memory_type_index = i;
      return true;
    }
  }

  LOG(ERROR) << "Could not find a valid memory index.";
  return false;
}

}  // namespace

Handle<VkDeviceMemory>
AllocMemory(Context* context, const VkMemoryRequirements& memory_reqs,
            const VkMemoryPropertyFlags& desired_properties) {
  uint32_t memory_type_index;
  if (!FindMemoryTypeIndex(context->physical_device_info.memory_properties,
                           memory_reqs.memoryTypeBits,
                           desired_properties, &memory_type_index)) {
    return {};
  }

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = memory_reqs.size;
  alloc_info.memoryTypeIndex = memory_type_index;

  VkDeviceMemory memory;
  if (!VK_CALL(vkAllocateMemory, *context->device, &alloc_info, nullptr,
                                 &memory)) {
    return {};
  }
  return {context, memory};
}

bool
AllocBuffer(Context* context, const AllocBufferConfig& config,
            MemoryBacked<VkBuffer>* out) {
  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = config.size;
  buffer_info.usage = config.usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkBuffer buffer;
  if (!VK_CALL(vkCreateBuffer, *context->device, &buffer_info, nullptr,
                               &buffer)) {
    return false;
  }
  Handle<VkBuffer> buffer_handle(context, buffer);

  VkMemoryRequirements memory_reqs;
  vkGetBufferMemoryRequirements(*context->device, buffer, &memory_reqs);

  Handle<VkDeviceMemory> memory_handle = AllocMemory(context, memory_reqs,
                                                     config.properties);
  if (!memory_handle.has_value())
    return false;

  if (!VK_CALL(vkBindBufferMemory, *context->device, buffer, *memory_handle, 0))
    return false;

  out->handle = std::move(buffer_handle);
  out->memory = std::move(memory_handle);

  return true;
}

bool CopyBuffer(Context* context, VkBuffer src_buffer, VkBuffer dst_buffer,
                VkDeviceSize size) {
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
    return false;
  }
  // In order to ensure freeing.
  Handle<VkCommandBuffer> command_buffer_handle(context, command_buffer);

  // This command buffer will be used only once.
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  if (!VK_CALL(vkBeginCommandBuffer, command_buffer, &begin_info))
    return false;

  VkBufferCopy buffer_copy = {};
  buffer_copy.size = size;
  buffer_copy.srcOffset = 0;
  buffer_copy.dstOffset = 0;
  vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &buffer_copy);

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

bool AllocImage(Context* context, const Image& src_image,
                const AllocImageConfig& config, MemoryBacked<VkImage>* out) {
  VkImageCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  create_info.imageType = ToVulkan(src_image.type);
  create_info.extent.width = src_image.width;
  create_info.extent.height = src_image.height;
  create_info.extent.depth = 1;
  create_info.mipLevels = 1;
  create_info.arrayLayers = 1;
  create_info.format = ToVulkan(src_image.format);
  create_info.tiling = config.tiling;
  create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  create_info.usage = config.usage;
  create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  create_info.samples = VK_SAMPLE_COUNT_1_BIT;
  create_info.flags = 0; // Optional.

  VkImage image_handle;
  if (!VK_CALL(vkCreateImage, *context->device, &create_info, nullptr,
                              &image_handle)) {
    return false;
  }
  MemoryBacked<VkImage> image;
  image.handle.Set(context, image_handle);

  VkMemoryRequirements memory_reqs;
  vkGetImageMemoryRequirements(*context->device, image_handle, &memory_reqs);

  auto memory_handle = AllocMemory(context, memory_reqs, config.properties);
  if (!memory_handle.has_value())
    return false;

  *out = std::move(image);
  return true;
}


}  // namespace vulkan
}  // namespace warhol
