// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/allocator.h"

#include "warhol/utils/assert.h"

namespace warhol {
namespace vulkan {

// Allocator -------------------------------------------------------------------

static bool
AllocateFromPools(Allocator*, uint32_t size, uint32_t align,
                  uint32_t memory_types, bool host_visible, Allocation*);

bool Allocate(Allocator* allocator, uint32_t size, uint32_t align,
              uint32_t mem_type, bool host_visible, Allocation* out) {
  // See if we can allocate from one of the existent pools.
  if (AllocateFromPools(allocator, size, align, mem_type, host_visible, out))
    return true;

  // Can't allocate from existent pools. Creating a new one.
  VkDeviceSize pool_size = host_visible ? allocator->host_visible_memory_size
                                        : allocator->device_local_memory_size;
  MemoryPool memory_pool = {};
  memory_pool.id = allocator->next_pool_id++;
  memory_pool.memory_type = mem_type;
  memory_pool.size = pool_size;
  memory_pool.host_visible = host_visible;
  if (!Init(&memory_pool))
    return false;

  allocator->pools.push_back(std::move(memory_pool));

  // Allocate directly from the pool. If we can't we simply fail.
  if (!Allocate(&memory_pool, size, align, out))
    return false;

  return true;
}

// Allocator::Helpers ----------------------------------------------------------

static bool
AllocateFromPools(Allocator*, uint32_t , uint32_t ,
                  uint32_t , bool , Allocation*) {
  NOT_IMPLEMENTED();
  return false;
}

// MemoryPool ------------------------------------------------------------------

bool Init(MemoryPool*) {
  NOT_IMPLEMENTED();
  return false;
}

bool Shutdown(MemoryPool*);

bool Allocate(MemoryPool*, uint32_t , uint32_t , Allocation* ) {
  NOT_IMPLEMENTED();
  return false;
}

bool Free(MemoryPool*, Allocation* out);

}  // namespace vulkan
}  // namespace warhol
