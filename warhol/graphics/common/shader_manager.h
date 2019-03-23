// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "warhol/graphics/common/defs.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/macros.h"

namespace warhol {

// Explicit enum used for binding draw routines.
enum class ShaderID : uint32_t {
  kCommon,
  kLast,
};
const char* ShaderIDToString(ShaderID);

// Type determines which attributes a shader has.
enum class ShaderAttributeLayout : uint32_t {
  kMesh,
  kLast,
};

// Types of bindings a shader has in a particular stage.
enum class ShaderBinding {
  kNone = 0,
  kUniformBuffer,
  kSampler,     // Texture sampler.
  kLast,
};
const char* ShaderBindingToString(ShaderBinding);

struct ShaderDescription {
  ShaderID id;
  ShaderAttributeLayout attribute_layout;

  ShaderBinding vertex_bindings[4] = {};
  ShaderBinding fragment_bindings[4] = {};

  const char* name;

  int vertex_shader_id = -1;      // ID of where the vertex shader is loaded.
  int fragment_shader_id = -1;    // ID of where the fragment shader is loaded.

  // Which uniforms this shader has.
  std::vector<ShaderUniform> uniforms;
};
ShaderDescription* GetShaderDescription(ShaderID);

struct UniformBuffer {
  using UniformValue = float[4];
  UniformValue values[(uint32_t)ShaderUniform::kLast];
};

struct ShaderManager {
  virtual ~ShaderManager();

  std::vector<ShaderDescription> shaders;

  UniformBuffer values[kMaxFrameBuffering];
  int current_frame = 0;
  bool valid = false;   // Valid from Init() to Shutdown().

  // Virtual interface.
  virtual void Init() { NOT_REACHED("Must override."); }
  virtual void Shutdown() { NOT_REACHED("Must override."); }
};

void InitShaderManager(ShaderManager*);

// |size| must be a multiple of 4.
void SetUniforms(ShaderUniform, float* values, size_t size);

void ShutdownShaderManager(ShaderManager*);

}  // namespace warhol
