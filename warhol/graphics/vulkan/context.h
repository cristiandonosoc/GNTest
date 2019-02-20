// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "warhol/graphics/vulkan/allocator.h"
#include "warhol/graphics/vulkan/def.h"
#include "warhol/graphics/vulkan/handle.h"
#include "warhol/graphics/vulkan/memory_utils.h"
#include "warhol/graphics/vulkan/staging_manager.h"

#include "warhol/math/math.h"
#include "warhol/utils/macros.h"
#include "warhol/utils/optional.h"

namespace warhol {

struct WindowManager;

namespace vulkan {

// The indices for given queues families within a particular physical device.
struct QueueFamilyIndices {
  int graphics = -1;
  int present = -1;
};

// Represents the capabilities a physical device can have for swap chains.
struct SwapChainCapabilities {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

struct PhysicalDeviceInfo {
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory_properties;
  QueueFamilyIndices queue_family_indices;
  SwapChainCapabilities swap_chain_capabilities;
};

// The actual formats used by a particular swap chain.
struct SwapChainDetails {
  VkSurfaceFormatKHR format;
  VkPresentModeKHR present_mode;
  VkExtent2D extent;
};

struct Context {
  DEFAULT_CONSTRUCTOR(Context);
  DELETE_COPY_AND_ASSIGN(Context);
  DEFAULT_MOVE_AND_ASSIGN(Context);
  DEFAULT_DESTRUCTOR(Context);

  int current_frame = 0;

  Handle<VkInstance> instance = {};
  std::vector<const char*> extensions;
  std::vector<const char*> validation_layers;

  Handle<VkDebugUtilsMessengerEXT> debug_messenger = {};

  Handle<VkSurfaceKHR> surface = {};

  // TODO(Cristian): How to specify features that we want to require.
  std::vector<const char*> device_extensions;
  VkPhysicalDevice physical_device = VK_NULL_HANDLE; // Freed with |instance|.
  PhysicalDeviceInfo physical_device_info;

  Handle<VkDevice> device = {};
  VkQueue graphics_queue = VK_NULL_HANDLE;  // Freed with |device|.
  VkQueue present_queue = VK_NULL_HANDLE;   // Freed with |device|.

  Handle<VkCommandPool> command_pool = {};
  Allocator allocator = {};
  StagingManager staging_manager = {};

  Handle<VkSwapchainKHR> swap_chain = {};
  SwapChainDetails swap_chain_details = {};

  VkImage images[Definitions::kNumFrames];    // Freed with |swap_chain|.
  Handle<VkImageView> image_views[Definitions::kNumFrames];

  VkFormat depth_format = VK_FORMAT_UNDEFINED;
  MemoryBacked<VkImage> depth_image;
  Handle<VkImageView> depth_image_view;
};

bool InitVulkanContext(Context*, WindowManager*);

// |extensions| and |validation_layers| must already be set within context.
// If successful, |context| will be correctly filled with a VkInstance.
bool CreateContext(Context* context);

// The Context must be valid from here on...

bool SetupDebugCall(Context*, PFN_vkDebugUtilsMessengerCallbackEXT callback);

// |device_extensions| must already be set for both these calls.

// |swap_chain_details| will be filled for the picked physical device.
bool PickPhysicalDevice(Context*);
bool CreateLogicalDevice(Context*);

bool InitResourceManagement(Context*);

// A |device| must be created already.
bool CreateSwapChain(Context*, Pair<uint32_t> screen_size);

bool CreateImageViews(Context*);

bool CreateCommandPool(Context*);

bool CreateDepthResources(Context*);

}  // namespace vulkan
}  // namespace warhol
