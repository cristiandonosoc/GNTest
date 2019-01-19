// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>
#include <vector>

#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/handle.h"
#include "warhol/graphics/vulkan/memory.h"
#include "warhol/utils/macros.h"
#include "warhol/utils/types.h"

namespace warhol {
namespace vulkan {

struct Context;

// TODO(Cristian): Use vkDOOM3 actual allocator, which uses memory types and
//                 allocation types.

// Allocation ------------------------------------------------------------------

// Return handle object from an allocation.
struct Allocation {
  bool host_visible() const { return data != nullptr; }

  uint32_t pool_id = Limits::kUint32Max;       // Which pool this came from.
  uint32_t block_id = Limits::kUint32Max;      // What memory block this is.

  // NOT-OWNING! Device memory associated with this block.
  // |VkDeviceMemory| is a handle (similar to a pointer).
  VkDeviceMemory device_memory = VK_NULL_HANDLE;

  VkDeviceSize offset = 0;          // Offset into the memory pool.
  VkDeviceSize size = 0;            // Size of the block.
  uint8_t* data = nullptr;          // If host visible, it's mapped here.
};

// MemoryBlock -----------------------------------------------------------------

// Represents an allocated (or free) memory block within a MemoryPool.
struct MemoryBlock {
  // In what state of allocation a block is.
  enum class AllocationType {
    kFree,
    kBuffer,
    kImage,
    kImageLinear,
    kImageOptional,
    kNone,
  };
  static const char* AllocationTypeToString(AllocationType);

  uint32_t id = Limits::kUint32Max;
  VkDeviceSize size = 0;
  VkDeviceSize offset = 0;
  /* Block* prev = nullptr; */
  /* Block* next = nullptr; */
  std::unique_ptr<MemoryBlock> prev = nullptr;
  std::unique_ptr<MemoryBlock> next = nullptr;
  AllocationType allocation_type = AllocationType::kNone;
};

// MemoryPool ------------------------------------------------------------------

// NOTE: For internal use only. Allocations should go through the allocator.
// TODO(Cristian): If this is private, perhaps it should go to the .cc

// Descripts in what state of allocation
struct MemoryPool {
  bool is_host_visible() const { return memory_usage != MemoryUsage::kGPUOnly; }

  DEFAULT_CONSTRUCTOR(MemoryPool);
  DELETE_COPY_AND_ASSIGN(MemoryPool);
  DEFAULT_MOVE_AND_ASSIGN(MemoryPool);

  ~MemoryPool();    // Calls shutdown(this);

  uint32_t id = Limits::kUint32Max;
  uint32_t memory_type_index = Limits::kUint32Max;
  MemoryUsage memory_usage = MemoryUsage::kNone;
  VkDeviceSize size = 0;            // In bytes.
  /* uint32_t align = 0; */

  VkDeviceSize allocated = 0;       // In bytes.
  Handle<VkDeviceMemory> memory = {};
  uint8_t* data = nullptr;

  // The pool is a linked list of allocated blocks.
  // If a block and its neighbour are free, they are merged into one block.
  /* Block* head = nullptr; */
  std::unique_ptr<MemoryBlock> head = nullptr;
  uint32_t next_block_id = Limits::kUint32Max;
};

struct InitMemoryPoolConfig {
  uint32_t id;
  uint32_t memory_type_index;
  MemoryUsage memory_usage;
  VkDeviceSize size;
};
bool Init(Context*, MemoryPool*, const InitMemoryPoolConfig&);
void Shutdown(MemoryPool*);
bool AllocateFrom(MemoryPool*, uint32_t size, uint32_t align, Allocation* out);
// TODO(Cristian): Can be |out| const?
bool Free(MemoryPool*, Allocation* out);

// Allocator -------------------------------------------------------------------

struct Allocator {
  uint32_t next_pool_id = Limits::kUint32Max;
  uint32_t garbage_index = Limits::kUint32Max;

  // How big should each pool be when created (in bytes);
  uint32_t device_local_memory_size = 0;
  uint32_t host_visible_memory_size = 0;

  std::vector<MemoryPool> pools;
  std::vector<Allocation> garbage_lists[kNumFrames];
};

bool Init(Allocator*);

struct AllocateConfig {
  uint32_t size;
  uint32_t align;
  uint32_t memory_type_bits;
  MemoryUsage memory_usage;
};
bool AllocateFrom(Context*, Allocator*, const AllocateConfig&, Allocation* out);
bool Free(Allocation*);
void EmptyGarbage(Allocator*);

}  // namespace vulkan
}  // namespace warhol
