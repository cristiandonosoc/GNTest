// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

#include "warhol/graphics/common/renderer.h"

namespace warhol {

// Represents the paths where each particular assert lives.
struct BasePaths {
  std::string texture;
  std::string shader;
};

BasePaths GetBasePaths(RendererType);
std::string GetTexturePath(BasePaths*, const std::string_view& texture_name);
std::string GetShaderPath(BasePaths*, const std::string_view& shader_path);

}  // namespace warhol
