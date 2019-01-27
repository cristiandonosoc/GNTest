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

  NOT_REACHED("Unknown MemoryUsage.");
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
      preferred |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      break;
    case MemoryUsage::kGPUToCPU:
      required |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      preferred |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                   VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
      break;
    case MemoryUsage::kNone:
      NOT_REACHED("No memory usage defined.");
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

VkMemoryPropertyFlags
GetPropertyFlagsFromMemoryTypeIndex(Context* context,
                                    uint32_t memory_type_index) {
  const auto& mem_properties = context->physical_device_info.memory_properties;
  ASSERT(memory_type_index < mem_properties.memoryTypeCount);

  auto flags = mem_properties.memoryTypes[memory_type_index].propertyFlags;
  return flags;
}

std::string MemoryTypeIndexToString(Context* context,
                                    uint32_t memory_type_index) {
  auto flags = GetPropertyFlagsFromMemoryTypeIndex(context, memory_type_index);

  std::stringstream ss;
  if (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    ss << "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ";
  if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    ss << "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, ";
  if (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    ss << "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ";
  if (flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
    ss << "VK_MEMORY_PROPERTY_HOST_CACHED_BIT, ";
  if (flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
    ss << "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, ";
  if (flags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
    ss << "VK_MEMORY_PROPERTY_PROTECTED_BIT, ";

  return ss.str();
}

}  // namespace vulkan
}  // namespace warhol
