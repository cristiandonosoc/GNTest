// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/shader_manager.h"

#include "warhol/assets/assets.h"
#include "warhol/utils/string.h"

namespace warhol {
namespace vulkan {

// Init ------------------------------------------------------------------------

// We need to init every know shader within the system.

namespace {

VulkanShader* FindShader(VulkanShaderManager* shader_manager, ShaderID shader_id) {
  for (auto& shader : shader_manager->loaded_shaders) {
    // Exit if it already exists.
    if (shader.description.id == shader_id)
      return &shader;
  }

  return nullptr;
}

VulkanShader LoadShader(ShaderID) {
  /* const char* shader_name = ShaderIDToString(shader_id); */

  /* auto vert_path = */
  /*     Assets::VulkanShaderPath(StringPrintf("%s.vert", shader_name)); */
  /* auto frag_path = */
  /*     Assets::VulkanShaderPath(StringPrintf("%s.frag", shader_name)); */

  return {};
}

void CreateDescriptorSetLayout(VulkanShader* ) {
}

void InitVulkanShaderManager(VulkanShaderManager* shader_manager) {
  for (size_t i = 0; i < (size_t)ShaderID::kLast; i++) {
    ShaderID shader_id = (ShaderID)i;
    if (FindShader(shader_manager, shader_id) != nullptr)
      continue;

    // Read shader.
    VulkanShader shader = LoadShader(shader_id);
    CreateDescriptorSetLayout(&shader);
    shader_manager->loaded_shaders.push_back(std::move(shader));
  }
}

}  // namespace

void VulkanShaderManager::Init() {
  InitVulkanShaderManager(this);
}

void VulkanShaderManager::Shutdown() {

}

}  // namespace vulkan
}  // namespace warhol
