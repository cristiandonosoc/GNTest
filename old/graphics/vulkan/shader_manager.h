// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/common/shader_manager.h"
#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/handle.h"

namespace warhol {
namespace vulkan {

struct VulkanShader {
  ShaderDescription description = {};

  VkShaderModule vertex_module = VK_NULL_HANDLE;
  VkShaderModule fragment_module = VK_NULL_HANDLE;

  VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
};

struct VulkanShaderManager : public ShaderManager {
  static constexpr size_t kMaxDescriptorSets = 2;

  Handle<VkDescriptorPool> descriptor_pools[kMaxFrameBuffering];
  Handle<VkDescriptorSet> descriptor_sets[kMaxFrameBuffering]
                                         [kMaxDescriptorSets];

  std::vector<VulkanShader> loaded_shaders;

  void Init() override;
  void Shutdown() override;
};

}  // namespace vulkan
}  // namespace warhol

