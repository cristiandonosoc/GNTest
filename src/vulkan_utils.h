// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "status.h"

namespace warhol {

// Overall context to run a vulkan app.
// Will destroy the managed resources (instance, messengers) on destruction.
struct VulkanContext {
  ~VulkanContext();

  VkInstance instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debug_messenger_handle = VK_NULL_HANDLE;
  VkPhysicalDevice device = VK_NULL_HANDLE;
  std::vector<VkPhysicalDevice> devices;
  // TODO(cristiandonosoc): This can unmap the queue from the device.
  //                        Should be keyed by it.
  int graphics_queue_index = -1;
};

// GetInstanceProcAddr calls ---------------------------------------------------

// This macro permits to rewrite the common pattern of getting the proc address
// and calling it with the correct semantic.

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


// Enum stringifying -----------------------------------------------------------

// Will be explicitly specialized in the .cc
template <typename VulkanEnum>
const char* VulkanEnumToString(VulkanEnum) {
  static_assert(false, "Unimplemented print for: " __PRETTY_FUNCTION__);
}

// Specializations
template<> const char* VulkanEnumToString(VkResult);
template<> const char* VulkanEnumToString(VkPhysicalDeviceType);

// Property getter -------------------------------------------------------------

// Wraps the overall pattern of asking how many are there and then getting them.

#define VK_GET_PROPERTIES(func, context, container) \
  {                                                 \
    uint32_t count = 0;                             \
    func((context), &count, nullptr);               \
    (container).resize(count);                        \
    func((context), &count, container.data());      \
  }

#define VK_GET_PROPERTIES_NC(func, container) \
  {                                                 \
    uint32_t count = 0;                             \
    func(&count, nullptr);               \
    (container).resize(count);                        \
    func(&count, (container).data());      \
  }



// Misc. -----------------------------------------------------------------------

#define VK_RETURN_IF_ERROR(result)                                 \
  if (result != VK_SUCCESS) {                                      \
    return Status("Vulkan Error: %s", VulkanEnumToString(result)); \
  }

}  // namespace warhol
