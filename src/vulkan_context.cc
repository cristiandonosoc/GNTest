// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "vulkan_context.h"
#include "vulkan_utils.h"

namespace warhol {

InstanceContext::InstanceContext() = default;
InstanceContext::~InstanceContext() {
  // We need to free the children first, as they have a reference to the vulkan
  // instance within this context.
  physical_devices.clear();

  if (handle == VK_NULL_HANDLE)
    return;

  // IMPORTANT: Keep the correct dependency order of destruction.
  if (debug_messenger_handle != VK_NULL_HANDLE)
    DestroyDebugUtilsMessengerEXT(handle, debug_messenger_handle, nullptr);

  if (surface != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(handle, surface, nullptr);

  vkDestroyInstance(handle, nullptr);
}

LogicalDeviceContext::LogicalDeviceContext() = default;
LogicalDeviceContext::~LogicalDeviceContext() {
  if (handle != VK_NULL_HANDLE)
    vkDestroyDevice(handle, nullptr);
}

}  // namespace warhol
