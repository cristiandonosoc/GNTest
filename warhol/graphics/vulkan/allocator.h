// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>
#include <vector>

#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/handle.h"
#include "warhol/graphics/vulkan/memory.h"
#include "warhol/utils/clear_on_move.h"
#include "warhol/utils/macros.h"
#include "warhol/utils/types.h"

namespace warhol {
namespace vulkan {

struct Allocator;     // Defined later in file.
struct Context;
struct MemoryPool;    // Defined later in file.

// TODO(Cristian): Use vkDOOM3 actual allocator, which uses memory types and
//                 allocation types.

// Allocation ------------------------------------------------------------------

// In what state of allocation a block is.
enum class AllocationType {
  kFree,
  kBuffer,
  kImage,
  kImageLinear,
  kImageOptimal,
  kNone,
};
static const char* AllocationTypeToString(AllocationType);

struct Allocation {
  bool valid() const { return pool != nullptr && has_value(); }
  bool host_visible() const { return data != nullptr; }
  bool has_value() const { return *memory!= VK_NULL_HANDLE; }

  VkDeviceMemory operator*() { return *memory; }

  DEFAULT_CONSTRUCTOR(Allocation);
  DELETE_COPY_AND_ASSIGN(Allocation);
  DEFAULT_MOVE_AND_ASSIGN(Allocation);
  ~Allocation();    // Calls Free(this) if valid().

  // IMPORTANT: |pool| and |pool_id| could differ if the pool was reallocated.
  MemoryPool* pool = nullptr;
  uint32_t pool_id = UINT32_MAX;
  uint32_t block_id = UINT32_MAX;      // What memory block this is.

  // NOT-OWNING! Device memory associated with this block.
  // |VkDeviceMemory| is a handle (similar to a pointer).
  ClearOnMove<VkDeviceMemory> memory = VK_NULL_HANDLE;

  VkDeviceSize offset = 0;          // Offset into the memory pool.
  VkDeviceSize size = 0;            // Size of the block.
  uint8_t* data = nullptr;          // If host visible, it's mapped here.
};

/* void CopyIntoAllocation(Allocation*, uint8_t* data, size_t size); */

/*******************************************************************************
 * MemoryPool & MemoryBlock implementation.
 *
 * This is the internal implementation about how the Allocator keeps track of
 * all the allocations.
 *
 * It basically is a list of MemoryPools. Each memory type will get a different
 * memory pool (device only, host visible, etc.).  Each pool is a normal block
 * allocator (linked list of blocks). When a pool doesn't have enough space,
 * a new one of that type will be allocated.
 *
 * When the allocations are freed, they are not really deallocated, but marked
 * for deletion for the current frame. Deletion lists are double buffered, so
 * that you're always deleting the other frames allocations.
 ******************************************************************************/

// MemoryBlock -----------------------------------------------------------------

// Represents an allocated (or free) memory block within a MemoryPool.
struct MemoryBlock {
  uint32_t id = UINT32_MAX;
  VkDeviceSize size = 0;        // |used_size| + alignment.
  VkDeviceSize used_size = 0;
  VkDeviceSize offset = 0;

  // A block owns the next block.
  MemoryBlock* prev = nullptr;
  MemoryBlock* next = nullptr;
  AllocationType alloc_type = AllocationType::kNone;
};

// MemoryPool ------------------------------------------------------------------

// NOTE: For internal use only. Allocations should go through the allocator.
// TODO(Cristian): If this is private, perhaps it should go to the .cc

// Descripts in what state of allocation
struct MemoryPool {
  bool valid() const { return memory.has_value(); }
  bool host_visible() const { return memory_usage != MemoryUsage::kGPUOnly; }
  VkDeviceSize free() const { return size - allocated; }

  DEFAULT_CONSTRUCTOR(MemoryPool);
  DELETE_COPY_AND_ASSIGN(MemoryPool);
  DEFAULT_MOVE_AND_ASSIGN(MemoryPool);
  ~MemoryPool();    // Calls Shutdown(this) if valid.

  Allocator* allocator = nullptr;   // Not owning.

  uint32_t id = UINT32_MAX;
  uint32_t memory_type_index = UINT32_MAX;
  MemoryUsage memory_usage = MemoryUsage::kNone;
  VkDeviceSize size = 0;            // In bytes.
  // Memory page alignment linear and optimal resources need to be apart with.
  // Comes from the Vulkan 1.0.39 spec. "Buffer-Image Granularity".
  // Also known as "Linear-Optimal Granularity".
  VkDeviceSize granularity = 0;

  VkDeviceSize allocated = 0;       // In bytes.
  Handle<VkDeviceMemory> memory = {};
  uint8_t* data = nullptr;

  // The pool is a linked list of allocated blocks.
  // If a block and its neighbour are free, they are merged into one block.
  // TODO(Cristian): Use a linked list structure instead of an adhoc one!
  std::unique_ptr<MemoryBlock> head = nullptr;
  uint32_t next_block_id = 0;

  // The allocations that have been returned.
  uint32_t garbage_index = 0;
  // They are block ids.
  struct GarbageMarker {
    uint32_t block_id;
    uint32_t size;
  };
  std::vector<GarbageMarker> garbage[Definitions::kNumFrames];
};

bool InitMemoryPool(Context*, MemoryPool*);

struct AllocateConfig;  // Defined later.
bool AllocateFromMemoryPool(Context*, MemoryPool*, const AllocateConfig&,
                            Allocation* out);

// This will invalidate *all* allocations.
void Shutdown(MemoryPool*);

// Takes ownership of the allocation.
void MarkForFree(MemoryPool*, Allocation*);
void EmptyGarbage(MemoryPool*);

bool IsHostCoherent(MemoryPool*);

std::string Print(Context*, MemoryPool*);

// Allocator -------------------------------------------------------------------

struct Allocator {
  bool valid() const { return initialized; }

  Allocator();
  ~Allocator();
  DELETE_COPY_AND_ASSIGN(Allocator);
  DEFAULT_MOVE_AND_ASSIGN(Allocator);

  Context* context = nullptr;   // not owning.

  uint32_t next_pool_id = UINT32_MAX;

  // How big should each pool be when created (in bytes);
  uint32_t device_local_memory_size = 0;
  uint32_t host_visible_memory_size = 0;
  bool initialized = false;

  // Unique pointers so that they don't move in memory.
  std::vector<std::unique_ptr<MemoryPool>> pools;
};

// The memory inputs are the sizes of the pools.
void InitAllocator(Context*, Allocator*, uint32_t device_local_memory,
                   uint32_t host_visible_memory);

struct AllocateConfig {
  uint32_t size = 0;
  uint32_t align = 0;
  uint32_t memory_type_bits = 0;
  MemoryUsage memory_usage = MemoryUsage::kNone;
  AllocationType alloc_type = AllocationType::kNone;
};
bool Allocate(Context*, Allocator*, const AllocateConfig&, Allocation* out);
// IMPORTANT: This will invalidate *all* allocations!
void Shutdown(Allocator*);

void MarkForFree(Allocator*, Allocation);

// Frees the garbage of the next frame to be used.
void EmptyGarbage(Allocator*);

std::string Print(Context*, Allocator*);

}  // namespace vulkan
}  // namespace warhol
