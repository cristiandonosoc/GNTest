// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>

#include "warhol/memory/memory_tracker.h"
#include "warhol/utils/log.h"
#include "warhol/utils/types.h"

namespace warhol {

struct MemoryPool {
  RAII_CONSTRUCTORS(MemoryPool);

  const char* name = nullptr;
  size_t size = 0;                // In bytes.
  uint8_t* current = nullptr;     // Where the next byte will be taken from.

  std::unique_ptr<uint8_t[]> data;

  TrackToken track_token;
};

inline uint8_t* Data(MemoryPool* pool) { return pool->data.get(); }

inline bool Valid(MemoryPool* pool) { return !!pool->data; }

inline bool Empty(MemoryPool* pool) { return pool->size == 0; }

// In bytes.
inline size_t Used(MemoryPool* pool) {
  ASSERT(Valid(pool));
  return pool->current - pool->data.get();
}

void InitMemoryPool(MemoryPool*, size_t bytes);

// Will not deallocate, but rather treat the memory as cleared.
void ResetMemoryPool(MemoryPool*);

// RAII semantics will take care of this also.
void ShutdownMemoryPool(MemoryPool*);

// Pushes arbitraty data into the memory pool.
uint8_t* Push(MemoryPool*, uint8_t* data, size_t size);

uint8_t* Reserve(MemoryPool*, size_t size);

template <typename T>
T* Push(MemoryPool* pool, T* data, size_t count) {
  return (T*)Push(pool, (uint8_t*)data, sizeof(T) * count);
}

template <typename T>
T* Push(MemoryPool* pool) {
  ASSERT(Valid(pool));
  ASSERT(pool->current + sizeof(T) < pool->data.get() + pool->size);

  T* value = (T*)pool->current;
  pool->current += sizeof(T);
  return value;
}

template <typename T>
T* Push(MemoryPool* pool, T t) {
  T* val = Push<T>(pool);
  *val = std::move(t);
  return val;
}

}  // namespace warhol
