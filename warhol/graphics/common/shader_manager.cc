// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/common/shader_manager.h"

#include "warhol/utils/assert.h"

namespace warhol {

const char* ShaderIDToString(ShaderID id) {
  switch (id) {
    case ShaderID::kCommon: return "Common";
    case ShaderID::kLast: return "Last";
  }
  NOT_REACHED("Unknown shader id.");
  return nullptr;
}

namespace {


}  // namespace

ShaderDescription* GetShaderDescription(ShaderID) {
  NOT_IMPLEMENTED();
  return nullptr;
}

// ShaderManager ---------------------------------------------------------------

ShaderManager::~ShaderManager() {
  if (valid)
    Shutdown();
}

}  // namespace warhol
