// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/allocator.h"

#include "warhol/graphics/vulkan/utils.h"
#include "warhol/graphics/vulkan/context.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {
namespace vulkan {

// MemoryBlock -----------------------------------------------------------------

const char*
MemoryBlock::AllocationTypeToString(MemoryBlock::AllocationType type) {
  switch (type) {
    case AllocationType::kFree: return "kFree";
    case AllocationType::kBuffer: return "kBuffer";
    case AllocationType::kImage: return "kImage";
    case AllocationType::kImageLinear: return "kImageLinear";
    case AllocationType::kImageOptional: return "kImageOptional";
    case AllocationType::kNone: break;
  }

  NOT_REACHED();
  return nullptr;
}

// MemoryPool ------------------------------------------------------------------

MemoryPool::~MemoryPool() { Shutdown(this); }

bool Init(Context* context, MemoryPool* pool,
          const InitMemoryPoolConfig& config) {
  pool->id = config.id;
  pool->memory_type_index = config.memory_type_index;
  pool->memory_usage = config.memory_usage;
  pool->size = config.size;

  auto memory = AllocMemory(context, pool->size, pool->memory_type_index);
  if (!memory.has_value())
    return false;

  pool->memory= std::move(memory);

  if (pool->is_host_visible()) {
    if (!VK_CALL(vkMapMemory, *context->device, *pool->memory, 0, pool->size,
                              0, (void**)&pool->data)) {
      return false;
    }
  }

  // Create an initial free block.
  auto head = std::make_unique<MemoryBlock>();
  *head = {};
  head->id = pool->next_block_id++;
  head->size = pool->size;
  head->offset = 0;
  head->allocation_type = MemoryBlock::AllocationType::kFree;

  pool->head = std::move(head);
  return true;
}

void Shutdown(MemoryPool* pool) {
  if (pool->is_host_visible())
    vkUnmapMemory(pool->memory.context()->device.value(), *pool->memory);
  pool->memory.Clear();

  // This will call all the chain destructors.
  pool->head.reset();

  // Once all the resources have been freed, we clear the values.
  *pool = {};
}

bool AllocateFrom(MemoryPool*, uint32_t , uint32_t , Allocation*) {
  NOT_IMPLEMENTED();
  return false;
}

bool Free(MemoryPool*, Allocation* out);

// Allocator -------------------------------------------------------------------

namespace {

bool AllocateFromPools(Allocator* allocator, const AllocateConfig& config,
                       uint32_t memory_type_index, Allocation* out) {
  for (MemoryPool& pool : allocator->pools) {
    if (pool.memory_type_index != memory_type_index)
      continue;

    if (AllocateFrom(&pool, config.size, config.align, out))
      return true;
  }

  // We could not allocate from any of the existent pools.
  return false;
}

}  // namespace

bool AllocateFrom(Context* context, Allocator* allocator,
                  const AllocateConfig& config, Allocation* out) {
  uint32_t memory_type_index = FindMemoryTypeIndex(
      context, config.memory_usage, config.memory_type_bits);
  if (memory_type_index == UINT32_MAX) {
    LOG(ERROR) << "Could not find a memory type index.";
    return false;
  }

  // See if we can allocate from one of the existent pools.
  if (AllocateFromPools(allocator, config, memory_type_index, out))
    return true;

  // Can't allocate from existent pools. Creating a new one.
  VkDeviceSize pool_size = (config.memory_usage == MemoryUsage::kGPUOnly)
                               ? allocator->host_visible_memory_size
                               : allocator->device_local_memory_size;

  MemoryPool memory_pool;
  InitMemoryPoolConfig init_config = {};
  init_config.id = allocator->next_pool_id++;
  init_config.memory_type_index = memory_type_index;
  init_config.memory_usage = config.memory_usage;
  init_config.size = pool_size;
  if (!Init(context, &memory_pool, init_config))
    return false;

  allocator->pools.push_back(std::move(memory_pool));

  // Allocate directly from the pool. If we can't we simply fail.
  if (!AllocateFrom(&memory_pool, config.size, config.align, out)) {
    LOG(ERROR) << "Could not allocated from a new pool.";
    return false;
  }

  return true;
}

namespace {


#if 0

bool AllocateFromPools(Context* context,
                       Allocator* allocator,
                       const AllocateConfig& config,
                       Allocation* out) {
  const auto& mem_properties = context->physical_device_info.memory_properties;

  VkMemoryPropertyFlags required = 0;  // Device local by default.
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

#endif

}  // namespace

}  // namespace vulkan
}  // namespace warhol
