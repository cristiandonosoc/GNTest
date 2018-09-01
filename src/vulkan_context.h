// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "utils/status.h"

struct SDL_Window;

namespace warhol {

struct VulkanContext {
  ~VulkanContext();

  // Instance
  struct {
    VkInstance handle = VK_NULL_HANDLE;
    std::vector<const char*> extensions;
    std::vector<const char*> validation_layers;
  } instance;

  // Physical Device
  struct {
    VkPhysicalDevice handle = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;

    int graphics_queue_index = -1;
    int present_queue_index = -1;
    std::vector<VkQueueFamilyProperties> qf_properties;
  } physical_device;

  VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
  VkSurfaceKHR surface = VK_NULL_HANDLE;

  // Logical Device
  struct {
    VkDevice handle = VK_NULL_HANDLE;
    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue present_queue = VK_NULL_HANDLE;

  } logical_device;

  // Swap Chain
  struct {
    /* std::vector<VkSurfaceFormatKHR> formats; */
    /* std::vector<VkPresentModeKHR> present_modes; */

    VkSwapchainKHR handle = VK_NULL_HANDLE;

    VkSurfaceCapabilitiesKHR capabilites;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
  } swap_chain;

  std::vector<VkImage> images;
  std::vector<VkImageView> image_views;
};

Status
InitVulkanContext(VulkanContext*);


}  // namespace warhol
