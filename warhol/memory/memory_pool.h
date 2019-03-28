// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>

#include "warhol/utils/assert.h"
#include "warhol/utils/types.h"

namespace warhol {

struct MemoryPool {
  size_t size = 0;                // In bytes.
  uint8_t* current = nullptr;     // Where the next byte will be taken from.

  std::unique_ptr<uint8_t[]> data;
};

inline bool Valid(MemoryPool* pool) { return !!pool->data; }

// In bytes.
size_t Used(MemoryPool* pool);

void InitMemoryPool(MemoryPool*, size_t bytes);

// Will not deallocate, but rather treat the memory as cleared.
void ResetMemoryPool(MemoryPool*);

// RAII semantics will take care of this also.
void ShutdownMemoryPool(MemoryPool*);

template <typename T>
T* PushIntoPool(MemoryPool* pool) {
  ASSERT(Valid(pool));
  ASSERT(pool->current + sizeof(T) < pool->data.get() + pool->size);

  T* value = (T*)pool->current;
  pool->current += sizeof(T);
  return value;
}



}  // namespace warhol
