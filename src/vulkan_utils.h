// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vulkan/vulkan.h>

#include "status.h"

namespace warhol {

template <typename VulkanEnum>
const char* VulkanEnumToString(VulkanEnum) {
  static_assert(false, "Unimplemented print for: " __PRETTY_FUNCTION__);
}

// Specializations
template<> const char* VulkanEnumToString(VkResult);
template<> const char* VulkanEnumToString(VkPhysicalDeviceType);

#define VK_RETURN_IF_ERROR(result)                                 \
  if (result != VK_SUCCESS) {                                      \
    return Status("Vulkan Error: %s", VulkanEnumToString(result)); \
  }

struct VulkanContext {
  ~VulkanContext();

  VkInstance instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debug_messenger_handle = VK_NULL_HANDLE;
  VkPhysicalDevice device = VK_NULL_HANDLE;
};

#define CREATE_VK_EXT_CALL(ext_name)                                           \
  template <typename... Args>                                                  \
  Status ext_name(VkInstance instance, Args &&... args) {                      \
    auto func =                                                                \
        (PFN_vk##ext_name)vkGetInstanceProcAddr(instance, "vk" #ext_name);     \
    if (!func) {                                                               \
      return Status("Extension " #ext_name " not present");                    \
    }                                                                          \
    func(instance, std::forward<Args>(args)...);                               \
    return Status();                                                           \
  }

CREATE_VK_EXT_CALL(CreateDebugUtilsMessengerEXT);
CREATE_VK_EXT_CALL(DestroyDebugUtilsMessengerEXT);

}  // namespace warhol
