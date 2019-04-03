// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <string>
#include <optional>
#include <vector>

#include "warhol/math/vec.h"
#include "warhol/memory/memory_pool.h"
#include "warhol/utils/clear_on_move.h"
#include "warhol/utils/macros.h"

namespace warhol {

struct Mesh;
struct RendererBackend;

struct Vertex {
  Vec3 pos;
  Vec3 color;
  Vec2 uv;
};

size_t Hash(const Vertex&);

struct Mesh {
  ClearOnMove<uint64_t> uuid = 0;     //  Zero is reserved.

  std::string name;

  MemoryPool vertices;
  MemoryPool indices;

  uint32_t vertex_size = 0;
  uint32_t vertex_count = 0;
  uint32_t index_count = 0;

  bool loaded = false;
};

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

// Thread safe. Will advance the UUID.
uint64_t GetNextMeshUUID();

}  // namespace warhol
