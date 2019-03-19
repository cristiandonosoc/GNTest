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
  size_t data_size() const { return vertices.size() * sizeof(Vertex); }

  Mesh();
  ~Mesh();
  DELETE_COPY_AND_ASSIGN(Mesh);
  DECLARE_MOVE_AND_ASSIGN(Mesh);

  uint64_t uuid = 0;     //  Zero is reserved.
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  // Represents a token RendererBackends will use to track down whether this
  // token has been loaded. 0 means non-loaded.
  bool loaded() const { return loaded_token != 0; }
  uint64_t loaded_token = 0;
};

std::optional<Mesh> LoadModel(const std::string& model_path);

}  // namespace warhol
