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

  struct Instance {
    VkInstance handle = VK_NULL_HANDLE;
    std::vector<const char*> extensions;
    std::vector<const char*> validation_layers;
  } instance;

  VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
  VkSurfaceKHR surface = VK_NULL_HANDLE;

  struct PhysicalDevice {
    VkPhysicalDevice handle = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;

    std::vector<const char*> extensions;

    // Queues
    int graphics_queue_index = -1;
    int present_queue_index = -1;
    std::vector<VkQueueFamilyProperties> qf_properties;

  } physical_device;

  struct LogicalDevice {
    VkDevice handle = VK_NULL_HANDLE;
    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue present_queue = VK_NULL_HANDLE;
  } logical_device;

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
  } swap_chain;

  struct Pipeline {
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    std::vector<VkShaderModule> shader_modules;
  } pipeline;

  std::vector<VkFramebuffer> frame_buffers;
  VkCommandPool command_pool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> command_buffers;

  Status Init(SDL_Window*);


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

  VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
  VkSemaphore render_finished_semaphore = VK_NULL_HANDLE;

  DELETE_COPY_AND_ASSIGN(VulkanContext);
};

}  // namespace warhol
