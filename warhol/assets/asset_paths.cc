// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/assets/asset_paths.h"

#include "warhol/utils/assert.h"
#include "warhol/utils/path.h"
#include "warhol/platform/platform.h"

namespace warhol {

std::string GetTexturePath(const std::string& texture_name) {
  return PathJoin({GetBasePath(), "assets", "textures", texture_name});
}

std::string GetShaderPath(const std::string& shader_name,
                          RendererType path_type) {
  const char* shader_dir = nullptr;
  switch (path_type) {
    case RendererType::kOpenGL:
      shader_dir = "opengl";
      break;
    case RendererType::kVulkan:
      shader_dir = "vulkan";
      break;
    case RendererType::kLast:
      NOT_REACHED("Invalid shader path type.");
  }

  return PathJoin({GetBasePath(), "assets", "shaders", shader_dir,
                   std::move(shader_name)});
}

}  // namespace warhol
