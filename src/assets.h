// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

namespace warhol {

// Grab-bag of functionality for assets handling.
class Assets {
 public:
  static std::string ShaderPath(std::string shader_name);
  static std::string TexturePath(std::string texture_name);
};

}  // namespace warhol
