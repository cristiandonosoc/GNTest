// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <vector>

#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/handle.h"
#include "warhol/utils/macros.h"
#include "warhol/utils/types.h"

namespace warhol {
namespace vulkan {

struct Allocation {
  uint32_t pool_id = Limits::kUint32Max;       // Which pool this came from.
  uint32_t block_id = Limits::kUint32Max;      // What memory block this is.

  // NOT-OWNING! Device memory associated with this block.
  // |VkDeviceMemory| is a handle (similar to a pointer).
  VkDeviceMemory device_memory = VK_NULL_HANDLE;

  VkDeviceSize offset = 0;          // Offset into the memory pool.
  VkDeviceSize size = 0;            // Size of the block.
  uint8_t* data = nullptr;          // If host visible, it's mapped here.
};

bool HostVisible(const Allocation& alloc) { return alloc.data != nullptr; }

// Allocator -------------------------------------------------------------------

struct MemoryPool;

struct Allocator {
  uint32_t next_pool_id = Limits::kUint32Max;
  uint32_t garbage_index = Limits::kUint32Max;

  // How big should each pool be when created (in bytes);
  uint32_t device_local_memory_size;
  uint32_t host_visible_memory_size;

  std::vector<MemoryPool> pools;
  std::vector<Allocation> garbage_lists[kNumFrames];
};

bool Init(Allocator*);
bool Allocate(Allocator*, uint32_t size, uint32_t align, uint32_t mem_type,
              bool host_visible, Allocation* out);
bool Free(Allocation*);
void EmptyGarbage(Allocator*);

// MemoryPool ------------------------------------------------------------------

// NOTE: For internal use only. Allocations should go through the allocator.

struct MemoryPool {
  DEFAULT_CONSTRUCTOR(MemoryPool);
  DEFAULT_DESTRUCTOR(MemoryPool);
  DELETE_COPY_AND_ASSIGN(MemoryPool);
  DEFAULT_MOVE_AND_ASSIGN(MemoryPool);

  // The pool is a linked list of allocated blocks.
  // If a block and its neighbour are free, they are merged into one block.
  struct Block {
    uint32_t id = Limits::kUint32Max;
    VkDeviceSize size = 0;
    VkDeviceSize offset = 0;
    Block* prev = nullptr;
    Block* next = nullptr;
    bool free = true;
  };

  uint32_t id = Limits::kUint32Max;
  uint32_t next_block_id = Limits::kUint32Max;
  uint32_t memory_type = Limits::kUint32Max;
  VkDeviceSize size = 0;            // In bytes.
  VkDeviceSize allocated = 0;       // In bytes.
  bool host_visible = false;
  Handle<VkDeviceMemory> device_memory = {};

  uint8_t* data = nullptr;
  Block* head = nullptr;
};

// TODO(Cristian): If this is private, perhaps it should go to the .cc
bool Init(MemoryPool*);
bool Shutdown(MemoryPool*);
bool Allocate(MemoryPool*, uint32_t size, uint32_t align, Allocation* out);
bool Free(MemoryPool*, Allocation* out);  // TODO(Cristian): Can be |out| const?

}  // namespace vulkan
}  // namespace warhol
