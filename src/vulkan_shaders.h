// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vulkan/vulkan.h>

#include "utils/macros.h"
#include "vulkan_context.h"

namespace warhol {

struct ShaderModuleContext {
  ~ShaderModuleContext();

  VulkanContext* context;   // Not owning.
  VkShaderModule handle = VK_NULL_HANDLE;

  DELETE_COPY_AND_ASSIGN(ShaderModuleContext);
};



}  // namespace warhol
