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

struct Uniform {
  std::string name;
  uint32_t size;      // In bytes.
};

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

  // The source will depend on what renderer backend is consuming this data.
  // TODO: Use a memory pool for this.
  std::vector<uint8_t> vert_source;
  std::vector<uint8_t> frag_source;
};

inline bool Valid(Shader* shader) { return shader->uuid.value != 0; }
inline bool Loaded(Shader* shader) {
  return !shader->vert_source.empty() && !shader->frag_source.empty();
}

// |shader_name| is the name identifier for this shader.
// |vert_name| and |frag_name| are the actual filenames of the shaders (without
// the .vert and .frag extensions).
bool LoadShader(BasePaths*, Renderer*,
                const std::string_view& shader_name,
                const std::string_view& vert_name,
                const std::string_view& frag_name,
                Shader* out);

// Same as the overload above, but assumes |vert_name| == |frag_name|, both
// being |filename|.
bool LoadShader(BasePaths*, Renderer*,
                const std::string_view& shader_name,
                const std::string_view& filename,
                Shader*);

void RemoveSources(Shader*);

std::string ShaderSourceAsString(const std::vector<uint8_t>& src);

// Thread safe. Will advance the UUID.
uint64_t GetNextShaderUUID();

}  // namespace warhol
