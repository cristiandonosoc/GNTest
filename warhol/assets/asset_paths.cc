// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/assets/asset_paths.h"

#include "warhol/platform/path.h"
#include "warhol/platform/platform.h"
#include "warhol/utils/log.h"

namespace warhol {

std::string GetTexturePath(const std::string_view& texture_name) {
  return PathJoin({GetBasePath(), "assets", "textures", texture_name});
}

std::string GetShaderPath(const std::string_view& shader_name,
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
      NOT_REACHED();
  }

  return PathJoin({GetBasePath(), "assets", "shaders", shader_dir,
                   std::move(shader_name)});
}

}  // namespace warhol
