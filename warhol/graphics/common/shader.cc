// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/shader.h"

#include "warhol/utils/assert.h"

namespace warhol {

const char* Shader::TypeToString (Shader::Type type) {
  switch (type) {
    case Shader::Type::kMesh: return "Mesh";
    case Shader::Type::kLast: return "Last";
  }

  NOT_REACHED("Unknown ShaderLayout.");
  return nullptr;
}

}  // namespace warhol
