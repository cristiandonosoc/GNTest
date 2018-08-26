// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>

#include "vulkan_context.h"
#include "vulkan_utils.h"

namespace warhol {

// InstanceContext ------------------------------------------------------------

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


  vkDestroyInstance(handle, nullptr);
}

// PhysicalDeviceContext ------------------------------------------------------

PhysicalDeviceContext::PhysicalDeviceContext(InstanceContext* instance)
    : instance(instance) {}
PhysicalDeviceContext::~PhysicalDeviceContext() {
  assert(instance);

  // Clear the logical devices first.
  logical_devices.clear();

  if (surface != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(instance->handle, surface, nullptr);
}

// LogicalDeviceContext -------------------------------------------------------

LogicalDeviceContext::LogicalDeviceContext(
    PhysicalDeviceContext* physical_device)
    : physical_device(physical_device) {}
LogicalDeviceContext::~LogicalDeviceContext() {
  // Clear the swap chains.
  swap_chain.reset();

  if (handle != VK_NULL_HANDLE)
    vkDestroyDevice(handle, nullptr);
}

// SwapChainContext -----------------------------------------------------------

SwapChainContext::SwapChainContext(LogicalDeviceContext* logical_device)
    : logical_device(logical_device) {}
SwapChainContext::~SwapChainContext() {
  assert(logical_device);
  if (handle != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(logical_device->handle, handle, nullptr);
}

}  // namespace warhol
