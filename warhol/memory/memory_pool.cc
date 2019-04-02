// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/memory/memory_pool.h"

namespace warhol {

void InitMemoryPool(MemoryPool* pool, size_t size) {
  ASSERT(!Valid(pool));
  pool->size = size;
  pool->data = std::make_unique<uint8_t[]>(size);
  pool->current = pool->data.get();
}

void ResetMemoryPool(MemoryPool* pool) {
  pool->current = pool->data.get();
}

MemoryPool::~MemoryPool() {
  if (Valid(this))
    ShutdownMemoryPool(this);
}

void ShutdownMemoryPool(MemoryPool* pool) {
  ASSERT(Valid(pool));

  pool->size = 0;
  pool->current = nullptr;
  pool->data.reset();
}

}  // namespace warhol
