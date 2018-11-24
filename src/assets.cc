// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/assets.h"

#include "src/platform/platform.h"
#include "src/utils/path.h"

namespace warhol {

std::string
Assets::ShaderPath(std::string shader_name) {
  return PathJoin({Platform::GetBasePath(),
                   "assets",
                   "shaders",
                   std::move(shader_name)});
}

std::string
Assets::TexturePath(std::string shader_name) {
  return PathJoin({Platform::GetBasePath(),
                   "assets",
                   "textures",
                   std::move(shader_name)});
}



}  // namespace warhol
