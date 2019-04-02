// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/memory/memory_pool.h"
#include "warhol/utils/clear_on_move.h"

namespace warhol {

// Represents an unstructured buffer agreement between the client and the
// renderer. Is is basically doing the same thing as loading a Mesh, but without
// having a presuposition of what the underlying data is.
//
// The renderer will blindly pass through the data, so it's the caller's
// responsability to make things work properly.
struct UnstructuredBuffer {
  ClearOnMove<uint64_t> uuid = 0;   // 0 is invalid.

  MemoryPool vert_data;
  MemoryPool index_data;
};

inline bool Valid(UnstructuredBuffer* b) { return b->uuid.has_value(); }

void InitUnstructuredBuffer(UnstructuredBuffer* buffer,
                            size_t vert_bytes,
                            size_t index_bytes);

void ShutdownUnstructuredBuffer(UnstructuredBuffer*);

// Thread safe. Will advance the UUID.
uint64_t GetNextUnstructuredBufferUUID();

}  // namespace warhol
