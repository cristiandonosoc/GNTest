// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "status.h"

struct SDL_Window;

namespace warhol {

// Context struct --------------------------------------------------------------

struct PhysicalDeviceContext;
struct LogicalDeviceContext;

// Overall context to run a vulkan app.
// Will destroy the managed resources (instance, messengers) on destruction.
struct InstanceContext {
  InstanceContext();
  ~InstanceContext();

  // Handles
  VkInstance handle = VK_NULL_HANDLE;
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debug_messenger_handle = VK_NULL_HANDLE;

  // Extensions to be added to the VkInstance
  std::vector<const char*> extensions;
  // List of validation layers.
  std::vector<const char*> validation_layers;

  std::vector<PhysicalDeviceContext> physical_devices;

  DELETE_COPY_AND_ASSIGN(InstanceContext);
  DECLARE_MOVE_AND_ASSIGN(InstanceContext);
};

struct PhysicalDeviceContext {
  VkPhysicalDevice handle = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  std::vector<VkQueueFamilyProperties> qf_properties;

  std::vector<LogicalDeviceContext> logical_devices;
};

struct LogicalDeviceContext {
  LogicalDeviceContext();
  ~LogicalDeviceContext();

  VkDevice handle = VK_NULL_HANDLE;

  // Queue in charge of the graphical commands.
  int graphics_queue_index = -1;
  VkQueue graphics_queue = VK_NULL_HANDLE;

  // Queue in charge of outputting to the system surface/window.
  int present_queue_index = -1;

  DELETE_COPY_AND_ASSIGN(LogicalDeviceContext);
  DECLARE_MOVE_AND_ASSIGN(LogicalDeviceContext);
};

// Vulkan Setup ----------------------------------------------------------------

// Creates an instance. The extensions and validation layers should be already
// set at this point.
Status SetupSDLVulkanInstance(InstanceContext*);

// Setups the logical devices and bionds the first one to the context.
Status SetupVulkanPhysicalDevices(InstanceContext*);

Status SetupVulkanLogicalDevices(InstanceContext*);

// Validation Layers -----------------------------------------------------------

// Gets the extensions SDL needs to hook up correctly with vulkan.
Status GetSDLExtensions(SDL_Window*, InstanceContext*);

// Validate that the requested layers are provided by the vulkan implementation.
bool CheckRequiredLayers(const std::vector<const char*>& requested_layers);


// GetInstanceProcAddr calls ---------------------------------------------------

// This macro permits to rewrite the common pattern of getting the proc address
// and calling it with the correct semantic.

#define CREATE_VK_EXT_CALL(ext_name)                                           \
  template <typename... Args>                                                  \
  Status ext_name(VkInstance instance, Args &&... args) {                      \
    auto func =                                                                \
        (PFN_vk##ext_name)vkGetInstanceProcAddr(instance, "vk" #ext_name);     \
    if (!func) {                                                               \
      return Status("Extension " #ext_name " not present");                    \
    }                                                                          \
    func(instance, std::forward<Args>(args)...);                               \
    return Status();                                                           \
  }

CREATE_VK_EXT_CALL(CreateDebugUtilsMessengerEXT);
CREATE_VK_EXT_CALL(DestroyDebugUtilsMessengerEXT);

// Property getter -------------------------------------------------------------

// Wraps the overall pattern of asking how many are there and then getting them.

#define VK_GET_PROPERTIES(func, context, container) \
  {                                                 \
    uint32_t count = 0;                             \
    func((context), &count, nullptr);               \
    (container).resize(count);                      \
    func((context), &count, container.data());      \
  }

#define VK_GET_PROPERTIES_NC(func, container) \
  {                                           \
    uint32_t count = 0;                       \
    func(&count, nullptr);                    \
    (container).resize(count);                \
    func(&count, (container).data());         \
  }

// Misc. -----------------------------------------------------------------------

#define VK_RETURN_IF_ERROR(result)                                 \
  if (result != VK_SUCCESS) {                                      \
    return Status("Vulkan Error: %s", VulkanEnumToString(result)); \
  }

// Enum stringifying -----------------------------------------------------------

// Will be explicitly specialized in the .cc
// NOTE: If not defined, this will be a linker error. Watch out for those.
template <typename VulkanEnum>
const char* VulkanEnumToString(VulkanEnum);

// Specializations. These are important because the definition is in the .cc
template<> const char* VulkanEnumToString(VkResult);
template<> const char* VulkanEnumToString(VkPhysicalDeviceType);

}  // namespace warhol
