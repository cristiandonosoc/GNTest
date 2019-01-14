// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/assets/assets.h"

#include "warhol/platform/platform.h"
#include "warhol/utils/path.h"

namespace warhol {

std::string Assets::ShaderPath(std::string shader_name) {
  return PathJoin({Platform::GetBasePath(), "assets", "shaders",
                   std::move(shader_name)});
}

std::string Assets::VulkanShaderPath(std::string shader_name) {
  return PathJoin({Platform::GetCurrentExecutableDirectory(),
                   "assets", "shaders", "vulkan", std::move(shader_name)});
}

std::string Assets::TexturePath(std::string shader_name) {
  return PathJoin({Platform::GetBasePath(), "assets", "textures",
                   std::move(shader_name)});
}

std::string Assets::ModelPath(std::string model_name) {
  return PathJoin({Platform::GetBasePath(), "assets", "models",
                   std::move(model_name)});
}

}  // namespace warhol
