// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <mutex>

#include "warhol/utils/types.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/macros.h"

namespace warhol {

enum class MemoryBlockSize {
  kNone = 0,
  k256 = KILOBYTES(1) / 4,
  k512 = KILOBYTES(1) / 2,
  k1K = KILOBYTES(1),
  k2K = KILOBYTES(2),
  k4K = KILOBYTES(4),
};

template <MemoryBlockSize BLOCK_SIZE, int BLOCK_COUNT>
struct ArrayHeap {
  struct DataBlock {
    uint8_t data[static_cast<size_t>(BLOCK_SIZE)];
  };

  struct MemoryBlock {
    MemoryBlock() = default;
    MemoryBlock(size_t size, uint8_t* data, ArrayHeap* heap)
        : size(size), data(data), heap(heap) {}
    DELETE_COPY_AND_ASSIGN(MemoryBlock);

    MemoryBlock(MemoryBlock&& rhs)
        : size(rhs.size), data(rhs.data), heap(rhs.heap) {
      rhs.Reset();
    }

    MemoryBlock& operator=(MemoryBlock&& rhs) {
      if (this != &rhs) {
        size = rhs.size;
        data = rhs.data;
        heap = rhs.heap;
        rhs.Reset();
      }
      return *this;
    }

    ~MemoryBlock() {
      bool freed = Free();
      ASSERT(freed);
    }

    bool Free() {
      if (!valid())
        return true;
      bool freed = heap->DeallocateBlock(*this);
      Reset();
      return freed;
    }

    void Reset() {
      size = 0;
      data = nullptr;
      heap = nullptr;
    }


    bool valid() const { return size > 0; }

    size_t size = 0;
    uint8_t* data = nullptr;
    ArrayHeap* heap = nullptr;
  };

  // The actual memory blocks this ArrayHeap holds.
  bool used[BLOCK_COUNT] = {};
  DataBlock blocks[BLOCK_COUNT] = {};
  std::mutex mutex;

  // API -----------------------------------------------------------------------

  MemoryBlock AllocateBlock() {
    // Atomically search for an open spot.
    int found_index = -1;
    {
      std::lock_guard<std::mutex> lock(mutex);
      for (size_t i = 0; i < BLOCK_COUNT; i++) {
        if (!used[i]) {
          used[i] = true;
          found_index = i;
          break;
        }
      }
    }

    ASSERT(found_index != -1);
    if (found_index != -1)
      return {(size_t)BLOCK_SIZE, (uint8_t*)&blocks[found_index], this};
    return {};
  }

  // Returns true if the block was found.
  bool DeallocateBlock(const MemoryBlock& block) {
    int found_index = -1;
    {
      std::lock_guard<std::mutex> lock(mutex);
      for (size_t i = 0; i < BLOCK_COUNT; i++) {
        if (block.data == (uint8_t*)&blocks[i]) {
          ASSERT(used[i]);
          used[i] = false;
          found_index = i;
          break;
        }
      }
    }

    return found_index != -1;
  }

};

}  // namespace warhol
