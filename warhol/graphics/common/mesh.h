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
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

inline size_t DataSize(Mesh* m) { return m->vertices.size() * sizeof(Vertex); }

std::optional<Mesh> LoadModel(const std::string& model_path);

bool LoadMesh(const std::string&, Mesh*);

// Will advance the UUID.
uint64_t GetNextMeshUUID();

}  // namespace warhol
