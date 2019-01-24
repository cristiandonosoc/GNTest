// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/allocator.h"

#include "warhol/containers/sort.h"
#include "warhol/graphics/vulkan/utils.h"
#include "warhol/graphics/vulkan/context.h"
#include "warhol/utils/align.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"

namespace warhol {
namespace vulkan {

// Allocation ------------------------------------------------------------------

namespace {

void MarkAsInvalid(Allocation* allocation) {
  allocation->pool = nullptr;
  allocation->pool_id = UINT32_MAX;
  allocation->block_id = UINT32_MAX;
  allocation->memory.clear();
  allocation->offset = 0;
  allocation->size = 0;
  allocation->data = nullptr;
}

}  // namespace

Allocation::~Allocation() {
  if (valid())
    Free(this);
}

void Free(Allocation* allocation) {
  // This can happen when Free was called not at the destroyer.
  if (!allocation->valid())
    return;

  // The MemoryPool could have gotten reallocated.
  ASSERT(allocation->pool->valid());
  ASSERT(allocation->pool->id == allocation->pool_id);

  MarkForFree(allocation->pool, allocation);
}

// MemoryBlock -----------------------------------------------------------------

const char*
AllocationTypeToString(AllocationType type) {
  switch (type) {
    case AllocationType::kFree: return "kFree";
    case AllocationType::kBuffer: return "kBuffer";
    case AllocationType::kImage: return "kImage";
    case AllocationType::kImageLinear: return "kImageLinear";
    case AllocationType::kImageOptimal: return "kImageOptimal";
    case AllocationType::kNone: return "kNone";
  }

  NOT_REACHED("Uknown AllocationType.");
  return nullptr;
}

// MemoryPool ------------------------------------------------------------------

namespace {

// This will check that 2 allocations (a & b) are bound to the same "page
// granurality". This granurality (represented here by |page_size|) is a page
// sized limit in which buffers and images must placed apart in order for them
// to not alias. This only applies when linear and optimal resources are layed
// out next to each other.
//
// The page size is in Physical Device Limits: Buffer Image Granularity.
//
// Algorithm comes from the Vulkan 1.0.39 spec. "Buffer-Image Granularity".
// Also known as "Linear-Optimal Granularity".
bool AreOnTheSamePage(VkDeviceSize a_offset, VkDeviceSize a_size,
                      VkDeviceSize b_offset, VkDeviceSize page_size) {
  // They cannot be literally coinciding in memory.
  ASSERT(a_offset + a_size <= b_offset && a_size > 0 && page_size > 0);


  VkDeviceSize a_end = a_offset + a_size - 1;
  // This aligns |a_end_page| to the granurality of the pages.
  VkDeviceSize a_end_page =  a_end & ~(page_size - 1);

  VkDeviceSize b_start = b_offset;
  VkDeviceSize b_start_page = b_start & (page_size - 1);

  return a_end_page == b_start_page;
}

// Granularity conflics occur when linear resources are allocated next to
// optimal resources.
bool HasGranularityConflict(AllocationType alloc_a,
                            AllocationType alloc_b) {
  if ((int)alloc_a > (int)alloc_b)
    SwapValues(alloc_a, alloc_b);

  switch (alloc_a) {
    case AllocationType::kFree:
      return false;
    case AllocationType::kBuffer:
      return alloc_b == AllocationType::kImage ||
             alloc_b == AllocationType::kImageOptimal;
    case AllocationType::kImage:
      return alloc_b == AllocationType::kImage ||
             alloc_b == AllocationType::kImageLinear ||
             alloc_b == AllocationType::kImageOptimal;
    case AllocationType::kImageLinear: return false;
      return alloc_b == AllocationType::kImageOptimal;
    case AllocationType::kImageOptimal:
      return false;
    case AllocationType::kNone:
      break;
  }

  NOT_REACHED("Invalid granularity check.");
  return false;
}

void Free(MemoryPool* pool, const MemoryPool::GarbageMarker& marker) {
  // Search for the block backing this allocation.
  MemoryBlock* current = pool->head.get();
  while (current) {
    if (current->id == marker.block_id)
      break;
    current = current->next;
  }

  if (current == nullptr) {
    LOG(ERROR) << "Could not find block " << marker.block_id << " in pool "
               << pool->id;
    NOT_REACHED("Unfound blog. Look in logs.");
  }

  current->alloc_type = AllocationType::kFree;

  // We see if we can merge them with the neighbouring blocks.
  if (current->prev && current->prev->alloc_type == AllocationType::kFree) {
    MemoryBlock* prev = current->prev;
    prev->next = current->next;
    // Point it back.
    if (current->next)
      current->next->prev = prev;

    prev->size = current->size;
    delete current;
    current = prev;
  }

  // See if we can merge it with the next block.
  if (current->next && current->next->alloc_type == AllocationType::kFree) {
    MemoryBlock* next = current->next;
    current->next = next->next;
    // Point it to the current.
    if (current->next)
      current->next->prev = current;
    current->size += next->size;
    delete next;
  }

  pool->allocated -= marker.size;
}

}  // namespace


MemoryPool::~MemoryPool() {
  if (valid())
    Shutdown(this);
}

bool Init(Context* context, MemoryPool* pool,
          const InitMemoryPoolConfig& config) {
  pool->id = config.id;
  pool->memory_type_index = config.memory_type_index;
  pool->memory_usage = config.memory_usage;
  pool->size = config.size;

  // TODO(Cristian): Handle granularity.";
  pool->granularity = 1;

  auto memory = AllocMemory(context, pool->size, pool->memory_type_index);
  if (!memory.has_value())
    return false;

  pool->memory= std::move(memory);

  if (pool->host_visible()) {
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
  head->alloc_type = AllocationType::kFree;

  pool->head = std::move(head);

  /* LOG(DEBUG) << "Head: " << pool->head.get(); */
  return true;
}

void Shutdown(MemoryPool* pool) {
  ASSERT(pool->valid());

  if (pool->host_visible())
    vkUnmapMemory(pool->memory.context()->device.value(), *pool->memory);
  pool->memory.Clear();

  // This will call all the chain destructors.
  pool->head.reset();

  // Once all the resources have been freed, we clear the values.
  *pool = {};
}

bool AllocateFromMemoryPool(Context* context, MemoryPool* pool,
                            const AllocateConfig& config, Allocation* out) {
  (void)context;
  VkDeviceSize free = pool->size - pool->allocated;
  /* LOG(DEBUG) << "Pool size: " << BytesToString(free) */
  /*            << ", required: " << BytesToString(config.size); */
  if (free < config.size)
    return false;

  MemoryBlock* current = nullptr;
  MemoryBlock* best_fit = nullptr;
  MemoryBlock* prev = nullptr;

  VkDeviceSize padding = 0;
  VkDeviceSize offset = 0;
  VkDeviceSize aligned_size = 0;

  /* LOG(DEBUG) << "Current head: " << pool->head.get(); */

  // We iterate over the linked list.
  for (current = pool->head.get();
       current != nullptr;
       prev = current, current = current->next) {
    /* LOG(DEBUG) << "Candidate block. Allocation Type: " */
    /*            << AllocationTypeToString(current->alloc_type) */
    /*            << ", size: " << BytesToString(current->size) << " KBs."; */

    // If it's allocated or too small, we continue searching.
    if (current->alloc_type != AllocationType::kFree ||
        current->size < config.size) {
      continue;
    }

    offset = Align(current->offset, config.align);

    // Check for linear/optimal granularity conflict with previous allocation.
    VkDeviceSize granularity = pool->granularity;
    ASSERT(granularity > 0);
    if (prev != nullptr && pool->granularity > 1 &&
        AreOnTheSamePage(prev->offset, prev->size, offset, pool->granularity) &&
        HasGranularityConflict(prev->alloc_type, config.alloc_type)) {
      offset = Align(current->offset, pool->granularity);
    }

    // How much difference between the end of this block and the start of the
    // aligned start.
    padding = offset - current->offset;
    aligned_size = padding + config.size;   // The actual block size we need.

    // If with the alignement the block is too small, we need to continue.
    if (aligned_size > current->size)
      continue;

    // If the alignment cases the pool to overflow, this pool won't cut it.
    if (aligned_size + pool->allocated > pool->size)
      return false;

    // If we're going to have problems with the next resource, we can't use it.
    if (granularity > 1 && current->next != nullptr) {
      MemoryBlock* next = current->next;
      if (AreOnTheSamePage(offset, config.size, next->offset, granularity) &&
          HasGranularityConflict(config.alloc_type, next->alloc_type)) {
        continue;
      }
    }

    best_fit = current;
    break;
  }

  // We couldn't find a valid block.
  if (best_fit == nullptr)
    return false;

  // We see if we need to split the block.
  // TODO(Cristian): Maybe define a minimum where we're interested in splitting
  //                 for. For 1KB, doesn't seem worth splitting.
  if (best_fit->size > config.size) {
    auto new_block = new MemoryBlock();   // TODO: Review this!
    *new_block = {};
    new_block->id = pool->next_block_id++;
    new_block->size = best_fit->size - aligned_size;
    new_block->offset = offset + config.size;
    new_block->alloc_type = AllocationType::kFree;

    // Insert it after |best_fit|.
    new_block->prev = best_fit;
    new_block->next = std::move(best_fit->next);
    if (new_block->next)
      new_block->next->prev = new_block;
    best_fit->next = new_block;
  }

  best_fit->alloc_type = config.alloc_type;
  best_fit->size = aligned_size;
  best_fit->used_size = config.size;

  LOG(DEBUG) << "Allocated block " << best_fit->id << " from pool " << pool->id;

  Allocation allocation = {};
  allocation.pool_id = pool->id;
  allocation.pool = pool;
  allocation.block_id = best_fit->id;
  allocation.memory = pool->memory.value();
  // NOTE: This could be different than the block offset.
  allocation.offset = offset;
  allocation.size = best_fit->used_size;
  if (pool->host_visible())
    allocation.data = pool->data + offset;

  *out= std::move(allocation);

  LOG(DEBUG) << "Allocated successfully: " << BytesToString(aligned_size);
  LOG(NO_FRAME) << Print(*context, *pool);

  pool->allocated += aligned_size;

  return true;
}

void MarkForFree(MemoryPool* pool, Allocation* allocation) {
  LOG(DEBUG) << "Pool " << pool->id << ": Marking for free block "
             << allocation->block_id;

  ASSERT(allocation->pool_id == pool->id);
  // We check that this allocation hasn't already been returned.
  for (auto& marker : pool->garbage[pool->garbage_index]) {
    if (marker.block_id == allocation->block_id) {
      LOG(ERROR) << "Pool " << pool->id << ": Attempting to free block "
                 << marker.block_id << " twice.";
      NOT_REACHED("Double allocation. Look in logs.");
    }
  }


  MemoryPool::GarbageMarker marker = {};
  marker.block_id = allocation->block_id;
  marker.size = allocation->size;
  pool->garbage[pool->garbage_index].push_back(std::move(marker));

  // This is now freed, so it's no longer to be marked valid.
  MarkAsInvalid(allocation);
}

void EmptyGarbage(MemoryPool* pool) {
  pool->garbage_index = (pool->garbage_index + 1) % kNumFrames;

  auto& garbage = pool->garbage[pool->garbage_index];
  for (auto& block_id : garbage) {
    Free(pool, block_id);
  }

  // Not free will be called again.
  garbage.clear();

  // If the pool is no longer allocates memory, we can deallocate it.
  if (pool->allocated == 0) {
    Shutdown(pool);
    ASSERT(!pool->valid());
  }
}

std::string Print(const Context& context, const MemoryPool& pool) {
  std::stringstream ss;
  ss << "Memory pool " << pool.id << " (Host Visible: " << pool.host_visible()
     << ")" << std::endl;
  ss << "Memory types: "
     << MemoryTypeIndexToString(context, pool.memory_type_index) << std::endl;
  ss << "Size: " << BytesToString(pool.size)
     << ", Allocated: " << BytesToString(pool.allocated)
     << ", Free: " << BytesToString(pool.free()) << std::endl;

  int count = 0;

  MemoryBlock* prev = nullptr;
  MemoryBlock* current = nullptr;
  for (current = pool.head.get(); current != nullptr; current = current->next) {
    prev = current->prev;

    ss << current->id
       << ". Type: " << AllocationTypeToString(current->alloc_type)
       << ", Size: " << BytesToString(current->size)
       << " (Used: " << BytesToString(current->used_size) << ")"
       << ", Offset: " << BytesToString(current->offset);

    if (prev != nullptr) {
      ss << ", Diff w/ prev: "
         << BytesToString(current->offset - (prev->offset + prev->size));
    }

    ss << std::endl;
    count++;
  }

  return ss.str();
}

// Allocator -------------------------------------------------------------------

namespace {

bool AllocateFromPools(Context* context, Allocator* allocator,
                       const AllocateConfig& config, uint32_t memory_type_index,
                       Allocation* out) {
  for (auto& pool : allocator->pools) {
    if (!pool->valid())
      continue;

    if (pool->memory_type_index != memory_type_index) {
      /* LOG(DEBUG) << "Different type index (pool: " << pool.memory_type_index */
      /*            << ", required: " << memory_type_index << ")."; */
      continue;
    }

    if (AllocateFromMemoryPool(context, pool.get(), config, out))
      return true;
  }

  // We could not allocate from any of the existent pools.
  return false;
}

MemoryPool* FindMemoryPool(Allocator* allocator, uint32_t pool_id) {
  for (auto& pool : allocator->pools) {
    if (pool->id == pool_id)
      return pool.get();
  }
  return nullptr;
}

}  // namespace

Allocator::~Allocator() = default;

void Init(Allocator* allocator, uint32_t device_local_memory,
          uint32_t host_visible_memory) {
  ASSERT(allocator->pools.empty());

  allocator->next_pool_id = 0;
  allocator->device_local_memory_size = device_local_memory;
  allocator->host_visible_memory_size = host_visible_memory;
  allocator->initialized = true;
}

void Shutdown(Allocator* allocator) {
  *allocator = {};
}

bool Allocate(Context* context, Allocator* allocator,
              const AllocateConfig& config, Allocation* out) {
  ASSERT(allocator->initialized);
  ASSERT(config.alloc_type != AllocationType::kNone);

  uint32_t memory_type_index = FindMemoryTypeIndex(
      context, config.memory_usage, config.memory_type_bits);
  ASSERT(memory_type_index != UINT32_MAX);

  /* LOG(DEBUG) << "Going to allocate from pools."; */

  // See if we can allocate from one of the existent pools.
  if (AllocateFromPools(context ,allocator, config, memory_type_index, out))
    return true;

  /* LOG(DEBUG) << "Could not allocate from any pool."; */
  SCOPE_LOCATION();

  // Can't allocate from existent pools. Creating a new one.
  VkDeviceSize pool_size = (config.memory_usage == MemoryUsage::kGPUOnly)
                               ? allocator->host_visible_memory_size
                               : allocator->device_local_memory_size;

  auto memory_pool = std::make_unique<MemoryPool>();
  *memory_pool = {};
  InitMemoryPoolConfig init_config = {};
  init_config.id = allocator->next_pool_id++;
  init_config.memory_type_index = memory_type_index;
  init_config.memory_usage = config.memory_usage;
  init_config.size = pool_size;
  if (!Init(context, memory_pool.get(), init_config))
    return false;

  /* LOG(DEBUG) << "Created memory pool. Current pool count: " << allocator->pools.size(); */

  // We search for a slot where to put this is.
  int inserted_index = -1;
  for (int i = 0; i < (int)allocator->pools.size(); i++) {
    auto& slot = allocator->pools[i];
    if (!slot->valid()) {
      slot = std::move(memory_pool);
      inserted_index = i;
      break;
    }
  }

  // If there wasn't any slot, we append it.
  if (inserted_index == -1) {
    inserted_index = (int)allocator->pools.size();
    allocator->pools.push_back(std::move(memory_pool));
  }

  LOG(DEBUG) << "Inserted pool in slot " << inserted_index;

  // Allocate directly from the pool. If we can't we simply fail.
  if (!AllocateFromMemoryPool(context, allocator->pools[inserted_index].get(),
                              config, out)) {
    LOG(ERROR) << "Could not allocate from a new pool.";
    return false;
  }

  return true;
}

void EmptyGarbage(Allocator* allocator) {
  for (auto& pool : allocator->pools) {
    EmptyGarbage(pool.get());
  }
}

std::string Print(const Context& context, const Allocator& allocator) {
  std::stringstream ss;
  ss << "Allocator status. Pools: " << allocator.pools.size() << std::endl;

  for (const auto& pool : allocator.pools) {
    ss << Print(context, *pool);
  }

  return ss.str();
}

}  // namespace vulkan
}  // namespace warhol
