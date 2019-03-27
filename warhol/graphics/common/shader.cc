// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/shader.h"

#include "warhol/utils/assert.h"
#include "warhol/utils/file.h"
#include "warhol/utils/string.h"

namespace warhol {

bool LoadShader(const std::string& path, Shader* shader) {
  std::string vert_path = StringPrintf("%s.vert", path.c_str());
  std::string vert_source;
  if (!ReadWholeFile(path, &vert_source))
    return false;

  std::string frag_path = StringPrintf("%s.frag", path.c_str());
  std::string frag_source;
  if (!ReadWholeFile(path, &frag_source))
    return false;

  shader->vert_source = std::move(vert_source);
  shader->frag_source = std::move(frag_source);
  return true;
}

void UnloadShader(Shader* shader) {
  shader->vert_source.clear();
  shader->frag_source.clear();
}

}  // namespace warhol
