// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vulkan/vulkan.h>

#include "status.h"

namespace warhol {

const char* VkResultToString(VkResult);

struct VulkanContext {
  ~VulkanContext();

  VkInstance instance_handle = nullptr;
  VkDebugUtilsMessengerEXT debug_messenger_handle = nullptr;
};

#define CREATE_VK_EXT_CALL(ext_name)                                 \
  template <typename... Args>                                                  \
  Status ext_name(VkInstance instance, Args &&... args) {                      \
    auto func = (PFN_vk ## ext_name)vkGetInstanceProcAddr(instance, "vk" #ext_name);     \
    if (!func) {                                                               \
      return Status("Extension " #ext_name " not present");                   \
    }                                                                          \
    func(instance, std::forward<Args>(args)...);                               \
    return Status();                                                           \
  }

CREATE_VK_EXT_CALL(CreateDebugUtilsMessengerEXT);
CREATE_VK_EXT_CALL(DestroyDebugUtilsMessengerEXT);

}  // namespace warhol
