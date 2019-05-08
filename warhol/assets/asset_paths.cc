// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/assets/asset_paths.h"

#include "warhol/platform/path.h"
#include "warhol/platform/platform.h"
#include "warhol/utils/log.h"

namespace warhol {

BasePaths GetBasePaths(RendererType type) {
  BasePaths paths;
  paths.texture = PathJoin({GetBasePath(), "assets", "textures"});

  const char* shader_dir = nullptr;
  switch (type) {
    case RendererType::kOpenGL:
      shader_dir = "opengl";
      break;
    case RendererType::kVulkan:
      shader_dir = "vulkan";
      break;
    case RendererType::kLast:
      NOT_REACHED();
  }
  paths.shader = PathJoin({GetBasePath(), "assets", "shaders", shader_dir});

  return paths;
}

std::string GetTexturePath(BasePaths* paths, const std::string_view& texture_name) {
  return PathJoin({paths->texture, texture_name});
}

std::string GetShaderPath(BasePaths* paths,
                          const std::string_view& shader_name) {
  return PathJoin({paths->shader, shader_name});
}

}  // namespace warhol
