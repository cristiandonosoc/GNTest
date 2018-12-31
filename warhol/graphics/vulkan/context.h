// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "warhol/math/math.h"
#include "warhol/utils/macros.h"
#include "warhol/utils/optional.h"

namespace warhol {
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
  Context() = default;
  ~Context();

  DELETE_COPY_AND_ASSIGN(Context);
  DEFAULT_MOVE_AND_ASSIGN(Context);

  Optional<VkDebugUtilsMessengerEXT> debug_messenger = {};

  Optional<VkInstance> instance = {};
  std::vector<const char*> extensions;
  std::vector<const char*> validation_layers;

  Optional<VkSurfaceKHR> surface = {};

  std::vector<const char*> device_extensions;
  VkPhysicalDevice physical_device = VK_NULL_HANDLE; // Freed with |instance|.
  PhysicalDeviceInfo device_info;

  Optional<VkDevice> device = {};
  VkQueue graphics_queue = VK_NULL_HANDLE;  // Freed with |device|.
  VkQueue present_queue = VK_NULL_HANDLE;   // Freed with |device|.

  Optional<VkSwapchainKHR> swap_chain = {};
  SwapChainDetails swap_chain_details = {};
  std::vector<VkImage> images;    // Freed with |swap_chain|.
  std::vector<VkImageView> image_views;
};

bool IsValid(const Context&);

// |extensions| and |validation_layers| must already be set within context.
// If successful, |context| will be correctly filled with a VkInstance.
bool CreateContext(Context* context);

// The Context must be valid from here on...

bool SetupDebugCall(Context*, PFN_vkDebugUtilsMessengerCallbackEXT callback);

// |device_extensions| must already be set for both these calls.

// |swap_chain_details| will be filled for the picked physical device.
bool PickPhysicalDevice(Context*);
bool CreateLogicalDevice(Context*);

// A |device| must be created already.
bool CreateSwapChain(Context*, Pair<uint32_t> screen_size);

bool CreateImageViews(Context*);

bool CreateGraphicsPipeline(Context*,
                            const std::string& vert_path,
                            const std::string& frag_path);

}  // namespace vulkan
}  // namespace warhol
