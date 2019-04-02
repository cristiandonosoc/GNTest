// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>

#include "warhol/math/vec.h"
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
  kNoop,
};

struct RenderCommandConfig {
  // Use a blend equation.
  bool blend_enabled = true;
  // Do face culling.
  bool cull_faces = true;
  // Use the depth buffer.
  bool depth_test = true;

  // Use scissor (only draw part of the framebuffer).
  bool use_scissor = false;
  Pair<int> scissor_p1;   // lower left corner.
  Pair<int> scissor_p2;   // Upper right corner.

  // Only draw wireframes.
  bool wireframe_mode = false;
};

struct RenderCommand {
  RenderCommandType type = RenderCommandType::kNoop;
  RenderCommandConfig config;

  Camera* camera;
  Shader* shader;
  union {
    LinkedList<MeshRenderAction>* mesh_actions;
  };
};

}  // namespace warhol
