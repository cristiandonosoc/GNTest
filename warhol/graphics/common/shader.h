// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

namespace warhol {


struct Uniform {
  std::string name;
  uint32_t size;      // In bytes.
};

struct Shader {
  uint64_t uuid = 0;  // Set up by the renderer.
  std::string name;
  std::string path;

  int vert_ubo_size = -1;
  std::vector<Uniform> vert_uniforms;

  int frag_ubo_size = -1;
  std::vector<Uniform> frag_uniforms;

  // Resetable state -----------------------------------------------------------

  std::string vert_source;
  std::string frag_source;
};

inline bool Valid(Shader* shader) { return shader->uuid != 0; }
inline bool Loaded(Shader* shader) {
  return !shader->vert_source.empty() && !shader->frag_source.empty();
}

// Will load the source, won't actually compile it.
// That happens on RendererUploadShader.
bool LoadShader(const std::string& name, const std::string& path, Shader*);

// Will only remove the data.
void UnloadShader(Shader*);

// Thread safe. Will advance the UUID.
uint64_t GetNextShaderUUID();

}  // namespace warhol
