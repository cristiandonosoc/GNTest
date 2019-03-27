// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/memory/memory_pool.h"

namespace warhol {

void InitMemoryPool(MemoryPool* pool, size_t size) {
  ASSERT(!Valid(pool));
  pool->size = size;
  pool->data = std::make_unique<uint8_t[]>(size);
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
