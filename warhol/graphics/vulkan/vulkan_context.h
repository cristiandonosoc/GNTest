// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "utils/status.h"

struct SDL_Window;

namespace warhol {

struct VulkanContext {
 public:
  VulkanContext();
  ~VulkanContext();

  // API  ----------------------------------------------------------------------

  Status Init(SDL_Window*);

  // Sub-types -----------------------------------------------------------------

  struct Instance {
    VkInstance handle = VK_NULL_HANDLE;
    std::vector<const char*> extensions;
    std::vector<const char*> validation_layers;
  };

  struct PhysicalDevice {
    VkPhysicalDevice handle = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;

    std::vector<const char*> extensions;

    // Queues
    int graphics_queue_index = -1;
    int present_queue_index = -1;
    std::vector<VkQueueFamilyProperties> qf_properties;

  };

  struct LogicalDevice {
    VkDevice handle = VK_NULL_HANDLE;
    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue present_queue = VK_NULL_HANDLE;
  };

  struct SwapChain {
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;

    VkSwapchainKHR handle = VK_NULL_HANDLE;

    VkSurfaceCapabilitiesKHR capabilites;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;

    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
  };

  struct Pipeline {
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    std::vector<VkShaderModule> shader_modules;
  };

 // Elements -------------------------------------------------------------------

  Instance instance;
  PhysicalDevice physical_device;
  LogicalDevice logical_device;
  SwapChain swap_chain;
  Pipeline pipeline;

  VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
  VkSurfaceKHR surface = VK_NULL_HANDLE;

  std::vector<VkFramebuffer> frame_buffers;
  VkCommandPool command_pool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> command_buffers;

  VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
  VkSemaphore render_finished_semaphore = VK_NULL_HANDLE;

 private:
  // Setup functions
  Status SetupInstance(SDL_Window*);
  Status SetupDebugMessenger();
  Status SetupPhysicalDevice();
  Status SetupSurface(SDL_Window*);
  Status SetupLogicalDevice();
  Status SetupSwapChain();
  Status SetupImages();
  Status SetupRenderPass();
  Status SetupPipelineLayout();
  Status SetupGraphicsPipeline();
  Status SetupFrameBuffers();
  Status SetupCommandPool();
  Status SetupCommandBuffers();
  Status BeginRenderPass();
  Status CreateSemaphores();

  DELETE_COPY_AND_ASSIGN(VulkanContext);
};

}  // namespace warhol
