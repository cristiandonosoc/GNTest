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

  // The counts of this are defined in shader.
  float* vert_values = nullptr;
  float* frag_values = nullptr;
  Texture* textures = nullptr;
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
