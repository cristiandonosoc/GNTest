// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include <warhol/utils/macros.h>
#include <warhol/utils/optional.h>
#include <vulkan/vulkan.h>

namespace warhol {
namespace vulkan {

struct QueueFamilyIndices {
  int graphics = -1;
  int present = -1;
};

struct SwapChainDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
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
  struct PhysicalDeviceInfo {
    QueueFamilyIndices queue_family_indices;
    SwapChainDetails swap_chain_details;
  } device_info;

  Optional<VkDevice> device = {};
  VkQueue graphics_queue = VK_NULL_HANDLE;  // Freed with |device|.
  VkQueue present_queue = VK_NULL_HANDLE;   // Freed with |device|.

  Optional<VkSwapchainKHR> swap_chain = {};
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
bool CreateSwapChain(Context*);

}  // namespace vulkan
}  // namespace warhol
