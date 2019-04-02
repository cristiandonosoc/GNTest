// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/unstructured_buffer.h"

#include <atomic>

namespace warhol {

namespace {

std::atomic<uint64_t> kNextBufferUUID = 1;

}  // namespace

uint64_t GetNextUnstructuredBufferUUID() {
  return kNextBufferUUID++;
}

void InitUnstructuredBuffer(UnstructuredBuffer* buffer,
                            size_t vert_bytes,
                            size_t index_bytes) {
  ASSERT(!Valid(buffer));

  buffer->uuid = GetNextUnstructuredBufferUUID();
  InitMemoryPool(&buffer->vert_data, vert_bytes);
  InitMemoryPool(&buffer->index_data, index_bytes);
}

void ShutdownUnstructuredBuffer(UnstructuredBuffer* buffer) {
  ShutdownMemoryPool(&buffer->vert_data);
  ShutdownMemoryPool(&buffer->index_data);
  buffer->uuid = 0;
}



}  // namespace warhol

