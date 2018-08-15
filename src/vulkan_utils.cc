// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>

#include <SDL2/SDL_vulkan.h>

#include "vulkan_utils.h"

namespace warhol {

// Context struct --------------------------------------------------------------

VulkanContext::~VulkanContext() {
  // IMPORTANT: Keep the correct dependency order of destruction.
  if (debug_messenger_handle)
    DestroyDebugUtilsMessengerEXT(instance, debug_messenger_handle, nullptr);
  if (instance)
    vkDestroyInstance(instance, nullptr);
}

// Validation Layers -----------------------------------------------------------

Status
GetSDLValidationLayers(SDL_Window* window,
                       std::vector<const char*>* extensions) {
  // Get the extensions needed by SDL
  VK_GET_PROPERTIES(SDL_Vulkan_GetInstanceExtensions, window, (*extensions));
  if (extensions->empty())
    return Status("Could not get SDL required extensions");

  return Status::Ok();
}

bool
CheckRequiredLayers(const std::vector<const char*>& requested_layers) {
  if (requested_layers.empty())
    return true;

  // Check available validation layers.
  std::vector<VkLayerProperties> available_layers;
  VK_GET_PROPERTIES_NC(vkEnumerateInstanceLayerProperties, available_layers);

  if (available_layers.empty())
    return false;

  // We check that the requested layers exist.
  for (const char* requested_layer : requested_layers) {
    bool layer_found = false;

    for (const auto& layer : available_layers) {
      if (strcmp(requested_layer, layer.layerName) == 0) {
        layer_found = true;
        break;
      }
    }

    if (!layer_found)
      return false;
  }

  return true;
};

// Enum stringifying -----------------------------------------------------------

template <>
const char*
VulkanEnumToString(VkResult res) {
  switch (res) {
    case VK_SUCCESS: return "SUCCESS";
    case VK_NOT_READY: return "NOT_READY";
    case VK_TIMEOUT: return "TIMEOUT";
    case VK_EVENT_SET: return "EVENT_SET";
    case VK_EVENT_RESET: return "EVENT_RESET";
    case VK_INCOMPLETE: return "INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return "ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST: return "ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED: return "ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT: return "ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT: return "ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return "ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS: return "ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED: return "ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL: return "ERROR_FRAGMENTED_POOL";
    case VK_ERROR_OUT_OF_POOL_MEMORY: return "ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
      return "ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_SURFACE_LOST_KHR: return "ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      return "ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR: return "SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR: return "ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      return "ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT: return "ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV: return "ERROR_INVALID_SHADER_NV";
    case VK_ERROR_FRAGMENTATION_EXT: return "ERROR_FRAGMENTATION_EXT";
    case VK_ERROR_NOT_PERMITTED_EXT:
      return "ERROR_NOT_PERMITTED_EXT";
    /* case VK_ERROR_OUT_OF_POOL_MEMORY_KHR: */
    /*   return "ERROR_OUT_OF_POOL_MEMORY_KHR"; */
    /* case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR: */
    /*   return "ERROR_INVALID_EXTERNAL_HANDLE_KHR"; */
    default: break;
  }

  assert(!"Unknown option");
  return "";
}

template <>
const char*
VulkanEnumToString(VkPhysicalDeviceType type) {
  switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "OTHER";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "INTEGRATED_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "DISCRETE_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "VIRTUAL_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
    default: break;
  }

  assert(!"Unknown option");
  return "";
}



}  // namespace warhol
