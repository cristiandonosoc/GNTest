// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <string>
#include <optional>
#include <vector>

#include "warhol/math/vec.h"
#include "warhol/memory/memory_pool.h"
#include "warhol/memory/memory_tracker.h"
#include "warhol/utils/clear_on_move.h"
#include "warhol/utils/log.h"
#include "warhol/utils/macros.h"
#include "warhol/utils/types.h"

namespace warhol {

struct Mesh;
struct RendererBackend;

struct Vertex {
  Vec3 pos;
  Vec3 color;
  Vec2 uv;
};

// The type also represents how many bytes the element is.
enum class AttributeType : uint32_t {
  kFloat = 4,
  kUint8 = 1,
  kLast,
};

struct Attribute {
  uint32_t count = 0;
  AttributeType type = AttributeType::kLast;

  // Whether the attribute data should be normalized when sent to the GPU.
  bool normalized = false;
};
inline uint32_t GetSize(Attribute* attribute) {
  return (uint32_t)attribute->type * attribute->count;
}

size_t Hash(const Vertex&);

struct Mesh {
  RAII_CONSTRUCTORS(Mesh);

  ClearOnMove<uint64_t> uuid = 0;     //  Zero is reserved.
  ClearOnMove<bool> staged = false;

  const char* name;

  MemoryPool vertices;
  MemoryPool indices;

  uint32_t vertex_size = 0;
  uint32_t vertex_count = 0;
  uint32_t index_count = 0;

  // Attributes are in order of how they appear in the shader layout.
  std::vector<Attribute> attributes;

  MemoryTrackToken<Mesh> track_token;
  bool loaded = false;
};

inline bool Staged(Mesh* mesh) { return mesh->staged.value; }

template <typename T>
inline void PushVertices(Mesh* mesh, T* data, size_t count) {
  ASSERT(mesh->vertex_size == sizeof(T));
  PushIntoMemoryPool(&mesh->vertices, data, count);
  mesh->vertex_count += count;
}

inline void PushIndices(Mesh* mesh, uint32_t* data, size_t count) {
  PushIntoMemoryPool(&mesh->indices, data, count);
  mesh->index_count += count;
}

inline void PushIndicesWithOffset(Mesh* mesh, uint32_t* data, size_t count,
                                  size_t offset) {
  MemoryPool* pool = &mesh->indices;
  uint32_t* src = data;
  uint32_t* dst = (uint32_t*)pool->current;
  ASSERT((uint8_t*)(dst + count) < pool->data.get() + pool->size);
  for (size_t i = 0; i < count; i++) {
    uint32_t val = *src++ + offset;
    *dst++ = val;
  }
  pool->current = (uint8_t*)dst;

  mesh->index_count += count;
}

bool LoadMesh(const std::string_view&, Mesh*);
void InitMeshPools(Mesh*, size_t vert_size, size_t index_size);

inline bool Valid(Mesh* mesh) { return mesh->uuid.value != 0; }
inline bool HasData(Mesh* mesh) {
  return Valid(&mesh->vertices) && Valid(&mesh->indices);
}

inline size_t VerticesSize(Mesh* mesh) {
  return mesh->vertex_count * mesh->vertex_size;
}

inline size_t IndicesSize(Mesh* mesh) {
  return mesh->index_count * sizeof(uint32_t);
}

uint32_t AttributesSize(Mesh* mesh);

// Thread safe. Will advance the UUID.
uint64_t GetNextMeshUUID();

}  // namespace warhol
