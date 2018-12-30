// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include <warhol/utils/macros.h>
#include <warhol/utils/optional.h>
#include <vulkan/vulkan.h>

namespace warhol {
namespace vulkan {

struct Context {
  Context() = default;
  ~Context();

  DELETE_COPY_AND_ASSIGN(Context);
  DEFAULT_MOVE_AND_ASSIGN(Context);

  Optional<VkDebugUtilsMessengerEXT> debug_messenger = {};

  Optional<VkInstance> instance = {};
  Optional<VkSurfaceKHR> surface = {};
  VkPhysicalDevice physical_device = VK_NULL_HANDLE; // Freed with |instance|.
  Optional<VkDevice> device = {};
  VkQueue graphics_queue = VK_NULL_HANDLE;  // Freed with |device|.
  VkQueue present_queue = VK_NULL_HANDLE;   // Freed with |device|.
};

bool IsValid(const Context&);

bool CreateContext(const std::vector<const char*>& extensions,
                   const std::vector<const char*>& layers,
                   Context* out);
bool SetupDebugCall(Context*, PFN_vkDebugUtilsMessengerCallbackEXT callback);

bool PickPhysicalDevice(Context*);
bool CreateLogicalDevice(Context*);


}  // namespace vulkan
}  // namespace warhol
