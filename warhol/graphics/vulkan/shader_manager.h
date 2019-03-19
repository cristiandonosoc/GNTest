// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/common/shader_manager.h"
#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/handle.h"


namespace warhol {
namespace vulkan {

struct VulkanShaderManager : public ShaderManager {
  static constexpr size_t kMaxDescriptorSets = 2;

  Handle<VkDescriptorPool> descriptor_pools[kMaxFrameBuffering];
  Handle<VkDescriptorSet> descriptor_sets[kMaxFrameBuffering]
                                         [kMaxDescriptorSets];

  void Init() override;
  void Shutdown() override;
};

}  // namespace vulkan
}  // namespace warhol

