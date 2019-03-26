// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/memory/memory_pool.h"

namespace warhol {

void InitMemoryPool(size_t bytes, MemoryPool* pool) {
  ASSERT(!Valid(pool));
  pool->size = bytes;
  pool->data = std::make_unique<uint8_t[]>(bytes);
}

void ResetMemoryPool(MemoryPool* pool) {
  pool->size = 0;
  pool->current = 0;
  pool->data.reset();
}

void ShutdownMemoryPool(MemoryPool* pool) {
  ResetMemoryPool(pool);
}

}  // namespace warhol
