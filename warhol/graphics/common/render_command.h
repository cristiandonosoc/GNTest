// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>

#include "warhol/containers/linked_list.h"
#include "warhol/graphics/common/texture.h"

namespace warhol {

struct Camera;
struct Mesh;
struct Shader;

struct MeshRenderAction {
  Mesh* mesh = nullptr;

  float* vert_values = nullptr;
  uint32_t vert_count = 0;    // Count of floats.

  float* frag_values = nullptr;
  uint32_t frag_count = 0;    // Count of floats.

  Texture* textures = nullptr;
  uint32_t texture_count = 0;
};

enum class RenderCommandType {
  kMesh,
  kLast,
};

struct RenderCommand {
  RenderCommandType type = RenderCommandType::kLast;

  Camera* camera;
  Shader* shader;
  union {
    LinkedList<MeshRenderAction>* mesh_actions;
  };
};

}  // namespace warhol
