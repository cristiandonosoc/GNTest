// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>

#include "warhol/utils/assert.h"
#include "warhol/utils/types.h"

namespace warhol {

struct MemoryPool {
  size_t pool_size = 0;

  // Where the next byte will be taken from.
  size_t current = 0;

  std::unique_ptr<uint8_t[]> data;
};

bool Valid(MemoryPool* pool) { return !!pool->data; }

void InitMemoryPool(size_t bytes, MemoryPool*);

// Will not deallocate, but rather treat the memory as cleared.
void ResetMemoryPool(MemoryPool*);

// RAII semantics will take care of this also.
void ShutdownMemoryPool(MemoryPool*);

template <typename T>
T* PushIntoPool(MemoryPool* pool) {
  ASSERT(Valid(pool));

  NOT_IMPLEMENTED();
}



}  // namespace warhol
