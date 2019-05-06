// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

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

bool LoadShader(const std::string_view& vert_name,
                const std::string_view& frag_name,
                Shader* out);

bool LoadShader(const std::string_view& name, Renderer*, Shader*);

void RemoveSources(Shader*);

std::string ShaderSourceAsString(const std::vector<uint8_t>& src);

// Thread safe. Will advance the UUID.
uint64_t GetNextShaderUUID();

}  // namespace warhol
