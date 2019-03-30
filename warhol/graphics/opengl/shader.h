// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace warhol {

struct Shader;

namespace opengl {

struct ShaderDescription {
  uint32_t program_handle = 0;

  // Uniform block bindings.
  // -1 means unbound.
  int camera_binding = -1;

  uint32_t vert_ubo_handle = 0;
  int vert_ubo_binding = -1;

  uint32_t frag_ubo_handle = 0;
  int frag_ubo_binding = -1;
};

// Uploads the shader and queries it's uniform locations.
bool UploadShader(Shader* shader, ShaderDescription* description);
void ShutdownShader(ShaderDescription*);

}  // namespace opengl
}  // namespace warhol
