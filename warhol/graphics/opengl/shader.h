// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include <string_view>

namespace warhol {

struct BasePaths;
struct Shader;

namespace opengl {

struct OpenGLRendererBackend;

struct ShaderHandles {
  uint32_t program_handle = 0;

  // Uniform block bindings.
  // -1 means unbound.
  int camera_binding = -1;

  uint32_t vert_ubo_handle = 0;
  int vert_ubo_binding = -1;

  uint32_t frag_ubo_handle = 0;
  int frag_ubo_binding = -1;
};

bool OpenGLParseShader(BasePaths*,
                       const std::string& vert_name,
                       const std::string& frag_name,
                       Shader* out);

bool OpenGLStageShader(OpenGLRendererBackend* opengl, Shader* shader);
void OpenGLUnstageShader(OpenGLRendererBackend* opengl, Shader* shader);

void DeleteShaderHandles(ShaderHandles* handles);

}  // namespace opengl
}  // namespace warhol
