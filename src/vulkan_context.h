// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "macros.h"

namespace warhol {

struct PhysicalDeviceContext;
struct LogicalDeviceContext;
struct SwapChainContext;
struct SwapChainProperties;

// Overall context to run a vulkan app.
// Will destroy the managed resources (instance, messengers) on destruction.
struct InstanceContext {
  InstanceContext();
  ~InstanceContext();

  // Handles
  VkInstance handle = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debug_messenger_handle = VK_NULL_HANDLE;

  // Extensions to be added to the VkInstance
  std::vector<const char*> extensions;
  // List of validation layers.
  std::vector<const char*> validation_layers;

  // The surface to where we will be drawing.

  // Store by unique ptr so that the move is trivial.
  // Otherwise care would be needed about moving the handles, as some compilers
  // won't bother invalidating the handle.
  // This is more relevant for the PhysicalDeviceContext though.
  std::vector<std::unique_ptr<PhysicalDeviceContext>> physical_devices;
  PhysicalDeviceContext* selected_physical_device;

  DELETE_COPY_AND_ASSIGN(InstanceContext);
};

struct SwapChainProperties {
  VkSurfaceCapabilitiesKHR capabilites;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

struct PhysicalDeviceContext {
  PhysicalDeviceContext(InstanceContext*);
  ~PhysicalDeviceContext();

  InstanceContext* instance;   // Not owning, must outline;

  VkPhysicalDevice handle = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;

  SwapChainProperties swap_chain_properties;

  // Queues
  int graphics_queue_index = -1;
  int present_queue_index = -1;
  std::vector<VkQueueFamilyProperties> qf_properties;

  // Surface.
  VkSurfaceKHR surface = VK_NULL_HANDLE;

  // Store by unique ptr so that the move is trivial.
  // Otherwise care would be needed about moving the handles, as some compilers
  // won't bother invalidating the handle.
  std::vector<std::unique_ptr<LogicalDeviceContext>> logical_devices;
  LogicalDeviceContext* selected_logical_device;

  DELETE_COPY_AND_ASSIGN(PhysicalDeviceContext);
};

struct LogicalDeviceContext {
  LogicalDeviceContext(PhysicalDeviceContext*);
  ~LogicalDeviceContext();

  PhysicalDeviceContext* physical_device;   // Not owning, must outline.

  VkDevice handle = VK_NULL_HANDLE;

  // Queue in charge of the graphical commands.
  VkQueue graphics_queue = VK_NULL_HANDLE;

  // Queue in charge of outputting to the system surface/window.
  VkQueue present_queue = VK_NULL_HANDLE;

  // Swapchain
  std::unique_ptr<SwapChainContext> swap_chain;

  DELETE_COPY_AND_ASSIGN(LogicalDeviceContext);
};

struct SwapChainContext {
  SwapChainContext(LogicalDeviceContext*);
  ~SwapChainContext();

  SwapChainProperties properties;

  VkSwapchainKHR handle;
  LogicalDeviceContext* logical_device;   // Not owning, must outlive.

  DELETE_COPY_AND_ASSIGN(SwapChainContext);
};



}  // namespace warhol
