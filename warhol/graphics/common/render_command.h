// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>

#include "warhol/math/vec.h"
#include "warhol/containers/linked_list.h"
#include "warhol/utils/log.h"

namespace warhol {

struct Camera;
struct Mesh;
struct Shader;
struct Texture;

// Index Range -----------------------------------------------------------------
//
// Packs size and offsets of the range.
// | size (32 bits) | offset (32 bits |

using IndexRange = uint64_t;

constexpr uint64_t kBottom32Mask = 0xffffffffu;
constexpr uint64_t kTop32Mask = kBottom32Mask << 32;

inline IndexRange PushOffset(IndexRange range, uint64_t offset) {
  uint64_t tmp = kTop32Mask | offset;
  return (range & kTop32Mask) | tmp;
}

inline uint32_t GetOffset(IndexRange range) {
  return kBottom32Mask & range;
}

inline IndexRange PushSize(IndexRange range, uint64_t size) {
  uint64_t tmp = (size << 32);
  return (range & kBottom32Mask) | tmp;
}

inline uint32_t GetSize(IndexRange range) {
  return range >> 32;
}

// RenderCommand ---------------------------------------------------------------

struct MeshRenderAction {
  Mesh* mesh = nullptr;

  Vec4 scissor;
  IndexRange indices;

  // The counts of this are defined in the corresponding shader.
  float* vert_uniforms = nullptr;
  float* frag_uniforms = nullptr;
  Texture* textures = nullptr;
};

enum class RenderCommandType {
  kMesh,
  kNoop,
  kLast,
};

struct RenderCommandConfig {
  // Use a blend equation.
  bool blend_enabled = true;
  // Do face culling.
  bool cull_faces = true;
  // Use the depth buffer.
  bool depth_test = true;

  // Only draw wireframes.
  bool wireframe_mode = false;
};

struct RenderCommand {
  RenderCommandType type = RenderCommandType::kLast;
  RenderCommandConfig config;

  Camera* camera;
  Shader* shader;
  union {
    LinkedList<MeshRenderAction>* mesh_actions;
  };
};

}  // namespace warhol
