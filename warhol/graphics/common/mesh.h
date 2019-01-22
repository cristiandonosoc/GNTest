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

struct Vertex {
  Vec3 pos;
  Vec3 color;
  Vec2 uv;
};

size_t Hash(const Vertex&);

struct Mesh {
  size_t data_size() const { return vertices.size() * sizeof(Vertex); }

  DEFAULT_CONSTRUCTOR(Mesh);
  DELETE_COPY_AND_ASSIGN(Mesh);
  DEFAULT_MOVE_AND_ASSIGN(Mesh);

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

std::optional<Mesh> LoadModel(const std::string& model_path);

}  // namespace warhol
