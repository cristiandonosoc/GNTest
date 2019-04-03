// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/memory/memory_pool.h"

#include "warhol/utils/log.h"

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

uint8_t* PushIntoMemoryPool(MemoryPool* pool, uint8_t* data, size_t size) {
  ASSERT(Valid(pool));
#ifndef NDEBUG
  if (pool->current + size > pool->data.get() + pool->size) {
    LOG(DEBUG) << "Overflowing pool!";
    LOG(DEBUG) << "Size: " << pool->size;
    LOG(DEBUG) << "Used: " << Used(pool)
               << " (diff: " << pool->size - Used(pool) << ").";
    LOG(DEBUG) << "Required: " << size;
    NOT_REACHED("Overflowing pool :(");
  }
#endif

  uint8_t* return_ptr = pool->current;
  uint8_t* ptr = pool->current;
  for (size_t i = 0; i < size; i++) {
    *ptr++ = *data++;
  }
  pool->current += size;
  return return_ptr;
}

}  // namespace warhol
