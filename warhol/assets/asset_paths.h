// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

namespace warhol {

std::string GetTexturePath(const std::string& texture_name);

enum class ShaderPathType {
  kOpenGL,
  kVulkan,
  kLast,
};
const char* ToString(ShaderPathType);

std::string GetShaderPath(const std::string& shader_name, ShaderPathType);

}  // namespace warhol
