// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "warhol/assets/asset_paths.h"
#include "warhol/utils/clear_on_move.h"
#include "warhol/utils/macros.h"

namespace warhol {

struct Renderer;

enum class UniformType {
  kBool, kInt, kFloat,
  kInt2, kInt3, kInt4,
  kUint2, kUint3, kUint4,
  kVec2, kVec3, kVec4,
  kIvec2, kIvec3, kIvec4,
  kUvec2, kUvec3, kUvec4,
  kMat4,
  kLast,
};
uint32_t GetSize(UniformType);
uint32_t GetAlignment(UniformType);

// Returns kLast if string is not valid.
UniformType FromString(const std::string&);

// |offset| represents the offset in memory where this uniform is from the
// beginning of the uniform block. This can be calculated calling into
// CalculateUniformLayout with a set of uniforms that have been loaded.
struct Uniform {
  std::string name;
  UniformType type = UniformType::kLast;
  uint32_t alignment = 0;   // In bytes.
  uint32_t offset = 0;      // In bytes.
  uint32_t size = 0;        // In bytes.
  // TODO(Cristian): Support arrays.
};

inline bool Valid(Uniform* u) { return u->type != UniformType::kLast; }

// This is using the std140 uniform block layout rules:
//
// https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL (Uniform block layout).
// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_uniform_buffer_object.txt
bool CalculateUniformLayout(std::vector<Uniform>* uniforms);

struct Shader {
  ClearOnMove<uint64_t> uuid = 0;  // Set up by the renderer.

  std::string name;
  std::string path;

  int vert_ubo_size = -1;     // In bytes.
  std::vector<Uniform> vert_uniforms;

  int frag_ubo_size = -1;     // In bytes.
  std::vector<Uniform> frag_uniforms;

  int texture_count = -1;

  // Resetable state -----------------------------------------------------------

  std::string vert_source;
  std::string frag_source;
};

inline bool Valid(Shader* shader) { return shader->uuid.value != 0; }
inline bool Loaded(Shader* shader) {
  return !shader->vert_source.empty() && !shader->frag_source.empty();
}

void RemoveSources(Shader*);

std::string ShaderSourceAsString(const std::vector<uint8_t>& src);

// Thread safe. Will advance the UUID.
uint64_t GetNextShaderUUID();

}  // namespace warhol
