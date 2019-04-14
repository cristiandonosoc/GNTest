// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <memory>

#include "warhol/math/vec.h"
#include "warhol/containers/list.h"
#include "warhol/utils/log.h"

namespace warhol {

struct Camera;
struct Mesh;
struct Shader;
struct Texture;

// Index Range -----------------------------------------------------------------
//
// Packs size and offsets of the range.
// | size (32 bits)  | offset (32 bits) |
//
// The easiest way to use this is to call CreateRange(size, offset).

using IndexRange = uint64_t;

constexpr uint64_t kBottom32Mask = 0xffffffffu;
constexpr uint64_t kTop32Mask = kBottom32Mask << 32;

inline IndexRange PushSize(IndexRange range, uint64_t size) {
  uint64_t tmp = (size << 32);
  return (range & kBottom32Mask) | tmp;
}

inline IndexRange PushOffset(IndexRange range, uint64_t offset) {
  uint64_t tmp = kBottom32Mask & offset;
  return (range & kTop32Mask) | tmp;
}

inline IndexRange CreateRange(uint32_t size, uint32_t offset) {
  IndexRange range = 0;
  range = PushSize(range, size);
  range = PushOffset(range, offset);
  return range;
}

inline uint32_t GetSize(IndexRange range) {
  return range >> 32;
}

inline uint32_t GetOffset(IndexRange range) {
  return kBottom32Mask & range;
}

std::string ToString(IndexRange);

// RenderCommand ---------------------------------------------------------------

struct MeshRenderAction {
  Mesh* mesh = nullptr;

  IntVec4 scissor;
  IndexRange index_range;

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
  bool blend_enabled = false;
  bool cull_faces = true;
  bool depth_test = true;
  bool scissor_test = false;
  bool wireframe_mode = false;
};

struct RenderCommand {
  DEFAULT_ALL_CONSTRUCTORS(RenderCommand);

  const char* name = nullptr;   // Useful for debugging purposes.

  RenderCommandType type = RenderCommandType::kLast;
  RenderCommandConfig config;

  Camera* camera;
  Shader* shader;
  union Actions {
    DECLARE_ALL_CONTRUCTORS(Actions);

    List<MeshRenderAction> mesh_actions;
  } actions;
};

}  // namespace warhol
