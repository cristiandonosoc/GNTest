// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/allocator.h"

#include "warhol/graphics/vulkan/context.h"
#include "warhol/utils/assert.h"

namespace warhol {
namespace vulkan {

// MemoryPool ------------------------------------------------------------------

bool Init(MemoryPool*, const InitMemoryPoolConfig&) {
  NOT_IMPLEMENTED();
  return false;
}

bool Shutdown(MemoryPool*);

bool AllocateFrom(MemoryPool*, uint32_t , uint32_t , Allocation*) {
  NOT_IMPLEMENTED();
  return false;
}

bool Free(MemoryPool*, Allocation* out);

// Allocator -------------------------------------------------------------------

namespace {

bool
AllocateFromPools(Context* context, Allocator* allocator,
                  const AllocateConfig& config, Allocation* out) {
  const auto& mem_properties = context->physical_device_info.memory_properties;

  VkMemoryPropertyFlags required = 0;   // Device local by default.
  if (config.host_visible) {
    required = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }

  // Device local is always preferred but not required.
  VkMemoryPropertyFlags preferred = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  // Iterate over pools looking for a preferred one.
  for (MemoryPool& pool : allocator->pools) {
    // If we need host visible and the pool doesn't suport it, continue.
    if (config.host_visible && !pool.host_visible)
      continue;

    // We try to match the memory index in the pool with what we require.
    if (((config.memory_types >> pool.memory_type_index) & 1) == 0)
      continue;

    // We check if it at least complies with the requirements.
    VkMemoryPropertyFlags properties =
        mem_properties.memoryTypes[pool.memory_type_index].propertyFlags;
    if ((properties & required) != required)
      continue;

    if ((properties & preferred) != preferred)
      continue;

    // Is what we require and prefer. We try to allocate from the pool.
    if (AllocateFrom(&pool, config.size, config.align, out))
      return true;
  }

  // Non that we prefer, we at least look for a required one.
  for (MemoryPool& pool : allocator->pools) {
    // If we need host visible and the pool doesn't suport it, continue.
    if (config.host_visible && !pool.host_visible)
      continue;

    // We try to match the memory index in the pool with what we require.
    if (((config.memory_types >> pool.memory_type_index) & 1) == 0)
      continue;

    // We check if it at least complies with the requirements.
    VkMemoryPropertyFlags properties =
        mem_properties.memoryTypes[pool.memory_type_index].propertyFlags;
    if ((properties & required) != required)
      continue;

    // Is what we require and prefer. We try to allocate from the pool.
    if (AllocateFrom(&pool, config.size, config.align, out))
      return true;
  }

  // We couldn't find a suitable memory pool.
  return false;
}

}  // namespace


bool Allocate(Context* context, Allocator* allocator,
              const AllocateConfig& config, Allocation* out) {
  // See if we can allocate from one of the existent pools.
  if (AllocateFromPools(context, allocator, config, out))
    return true;

  // Can't allocate from existent pools. Creating a new one.
  VkDeviceSize pool_size = config.host_visible
                               ? allocator->host_visible_memory_size
                               : allocator->device_local_memory_size;

  MemoryPool memory_pool;
  InitMemoryPoolConfig init_config = {};
  init_config.id = allocator->next_pool_id++;
  init_config.memory_types = config.memory_types;
  init_config.size = pool_size;
  init_config.host_visible = config.host_visible;
  if (!Init(&memory_pool, init_config))
    return false;

  allocator->pools.push_back(std::move(memory_pool));

  // Allocate directly from the pool. If we can't we simply fail.
  if (!AllocateFrom(&memory_pool, config.size, config.align, out))
    return false;

  return true;
}

}  // namespace vulkan
}  // namespace warhol
