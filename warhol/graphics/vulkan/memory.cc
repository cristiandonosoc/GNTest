// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/memory.h"

#include "warhol/graphics/vulkan/context.h"
#include "warhol/graphics/vulkan/commands.h"
#include "warhol/graphics/vulkan/image_utils.h"
#include "warhol/graphics/vulkan/utils.h"
#include "warhol/utils/scope_trigger.h"

namespace warhol {
namespace vulkan {

const char* MemoryUsageToString(MemoryUsage memory_usage) {
  switch (memory_usage) {
    case MemoryUsage::kNone: return "None";
    case MemoryUsage::kGPUOnly: return "GPUOnly";
    case MemoryUsage::kCPUOnly: return "CPUOnly";
    case MemoryUsage::kCPUToGPU: return "CPUToGPU";
    case MemoryUsage::kGPUToCPU: return "GPUToCPU";
  }

  NOT_REACHED();
  return nullptr;
}

// DEPRECATED.
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

uint32_t FindMemoryTypeIndex(Context* context, MemoryUsage memory_usage,
                             uint32_t memory_type_bits) {
  const auto& mem_properties = context->physical_device_info.memory_properties;

  VkMemoryPropertyFlags required = 0;
  VkMemoryPropertyFlags preferred = 0;

  switch (memory_usage) {
    case MemoryUsage::kGPUOnly:
      preferred |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      break;
    case MemoryUsage::kCPUOnly:
      required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      break;
    case MemoryUsage::kCPUToGPU:
      required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      preferred |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      break;
    case MemoryUsage::kGPUToCPU:
      required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      preferred |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                   VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
      break;
    case MemoryUsage::kNone:
      NOT_REACHED();
  }

  // Search for required & preferred.
  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    // See if we match in the memory type.
    if (((memory_type_bits >> i) & 1) == 0)
      continue;

    auto properties = mem_properties.memoryTypes[i].propertyFlags;
    if ((properties & required) != required ||
        (properties & preferred) != preferred) {
      continue;
    }
    return i;
  }

  // Search only for required.
  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    // See if we match in the memory type.
    if (((memory_type_bits >> i) & 1) == 0)
      continue;

    auto properties = mem_properties.memoryTypes[i].propertyFlags;
    if ((properties & required) != required)
      continue;
    return i;
  }

  return UINT32_MAX;
}

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

// VkBuffer --------------------------------------------------------------------

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

// VkImage ---------------------------------------------------------------------

bool CopyBufferToImage(Context* context, const Image& image,
                       VkBuffer src, VkImage dst) {
  auto command_buffer = BeginSingleTimeCommands(context);
  if (!command_buffer.has_value())
    return false;

  VkBufferImageCopy copy = {};
  copy.bufferOffset = 0;
  copy.bufferRowLength = 0;
  copy.bufferImageHeight = 0;
  copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  copy.imageSubresource.mipLevel = 0;
  copy.imageSubresource.baseArrayLayer = 0;
  copy.imageSubresource.layerCount = 1;
  copy.imageOffset = {0, 0, 0};
  copy.imageExtent = { (uint32_t)image.width, (uint32_t)image.height, 1 };

  vkCmdCopyBufferToImage(*command_buffer, src, dst,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

  if (!EndSingleTimeCommands(context, *command_buffer))
    return false;
  return true;
}

}  // namespace vulkan
}  // namespace warhol
