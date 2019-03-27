// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <string>
#include <optional>
#include <vector>

#include "warhol/math/vec.h"
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
  uint64_t uuid = 0;     //  Zero is reserved.

  std::string name;

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

inline bool Valid(Mesh* mesh) { return mesh->uuid != 0; }
inline bool Loaded(Mesh* mesh) {
  return !mesh->vertices.empty() && !mesh->indices.empty();
}

inline size_t VerticesSize(Mesh* mesh) {
  return mesh->vertices.size() * sizeof(Vertex);
}

inline size_t IndicesSize(Mesh* mesh) {
  return mesh->indices.size() * sizeof(uint32_t);
}

bool LoadMesh(const std::string&, Mesh*);

// Thread safe. Will advance the UUID.
uint64_t GetNextMeshUUID();

}  // namespace warhol
