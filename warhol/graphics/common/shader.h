// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

namespace warhol {


struct Shader {
  uint64_t uuid = 0;  // Set up by the renderer.
  std::string name;

  std::string vert_source;
  std::string frag_source;
};

// Will load the source, won't actually compile.
bool LoadShader(const std::string& path, Shader*);
// Will only remove the data.
void UnloadShader(Shader*);

bool HasSource(Shader*);

// Will advance the UUID.
uint64_t GetNextShaderUUID();

}  // namespace warhol
