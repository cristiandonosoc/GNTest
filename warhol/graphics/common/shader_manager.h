// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "warhol/graphics/common/defs.h"
#include "warhol/utils/assert.h"
#include "warhol/utils/macros.h"

namespace warhol {

enum class ShaderType : uint32_t ;
enum class ShaderUniform : uint32_t;

struct Shader {
  ShaderType type;
  const char* name;

  // Which uniforms this shader has.
  std::vector<ShaderUniform> uniforms;
};

// Explicit enum used for binding draw routines.
enum class ShaderID {
  kCommon,
  kLast,
};
const char* ShaderIDToString(ShaderID);

// A Shader can have any combination of this uniforms.
// The set is determined by the shader uniform layout.
enum class ShaderUniform : uint32_t {
  kModelMatrixX,
  kModelMatrixY,
  kModelMatrixZ,
  kModelMatrixW,

  kViewMatrixX,
  kViewMatrixY,
  kViewMatrixZ,
  kViewMatrixW,

  kProjectionMatrixX,
  kProjectionMatrixY,
  kProjectionMatrixZ,
  kProjectionMatrixW,

  kTex0U, kTex0V,
  kTex1U, kTex1V,

  kUser0,
  kUser1,
  kUser2,
  kUser3,
  kUser4,
  kUser5,
  kUser6,
  kUser7,

  kLast,
};


struct UniformBuffer {
  using UniformValue = float[4];
  UniformValue values[(uint32_t)ShaderUniform::kLast];
};

struct ShaderManager {
  virtual ~ShaderManager();

  std::vector<Shader> shaders;
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

// Type determines which attributes a shader has.
enum class ShaderType : uint32_t {
  kMesh,
  kLast,
};



}  // namespace warhol
