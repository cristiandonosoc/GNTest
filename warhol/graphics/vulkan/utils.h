// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "warhol/utils/log.h"

namespace warhol {

// GetInstanceProcAddr calls ---------------------------------------------------

// This macro permits to rewrite the common pattern of getting the proc address
// and calling it with the correct semantic.

#define CREATE_VK_EXT_CALL(ext_name)                                       \
  template <typename... Args>                                              \
  bool ext_name(VkInstance instance, Args&&... args) {                     \
    auto func =                                                            \
        (PFN_vk##ext_name)vkGetInstanceProcAddr(instance, "vk" #ext_name); \
    if (!func) {                                                           \
      LOG(WARNING) << "Extension " << #ext_name << " not present";         \
      return false;                                                        \
    }                                                                      \
    func(instance, std::forward<Args>(args)...);                           \
    return true;                                                           \
  }

CREATE_VK_EXT_CALL(CreateDebugUtilsMessengerEXT);
CREATE_VK_EXT_CALL(DestroyDebugUtilsMessengerEXT);

// Property getter -------------------------------------------------------------

// Wraps the overall pattern of asking how many are there and then getting them.

#define VK_GET_PROPERTIES(func, context, container) \
  {                                                 \
    uint32_t count = 0;                             \
    func((context), &count, nullptr);               \
    (container).resize(count);                      \
    func((context), &count, container.data());      \
  }

#define VK_GET_PROPERTIES_NC(func, container) \
  {                                           \
    uint32_t count = 0;                       \
    func(&count, nullptr);                    \
    (container).resize(count);                \
    func(&count, (container).data());         \
  }

// Enum stringifying -----------------------------------------------------------

// Will be explicitly specialized in the .cc
// NOTE: If not defined, this will be a linker error. Watch out for those.
template <typename VulkanEnum>
const char* VulkanEnumToString(VulkanEnum);

// Specializations. These are important because the definition is in the .cc
template<> const char* VulkanEnumToString(VkResult);
template<> const char* VulkanEnumToString(VkPhysicalDeviceType);

}  // namespace warhol
