// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/memory.h"

#include "warhol/graphics/vulkan/context.h"

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

}  // namespace vulkan
}  // namespace warhol
