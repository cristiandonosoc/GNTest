// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <string.h>

#include <SDL2/SDL_Vulkan.h>

#include "vulkan_context.h"
#include "vulkan_utils.h"

namespace warhol {

// TODO: Setup a better debug call.
static VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCall(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                VkDebugUtilsMessageTypeFlagsEXT type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                void* user_data) {
  (void)severity;
  (void)type;
  (void)user_data;
  printf("Validation layer message: %s\n", callback_data->pMessage);

  return VK_FALSE;
}



VulkanContext::~VulkanContext() {
  // We destroy elements backwards from allocation.
  for (VkImageView& image_view : image_views) {
    if (image_view != VK_NULL_HANDLE) {
      vkDestroyImageView(logical_device.handle, image_view, nullptr);
    }
  }

  if (swap_chain.handle != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(logical_device.handle, swap_chain.handle, nullptr);

  if (logical_device.handle != VK_NULL_HANDLE)
    vkDestroyDevice(logical_device.handle, nullptr);

  if (surface != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(instance.handle, surface, nullptr);

  if (debug_messenger != VK_NULL_HANDLE) {
    printf("LOG: Destroying debug messenger\n");
    DestroyDebugUtilsMessengerEXT(instance.handle, debug_messenger, nullptr);
  }

  if (instance.handle != VK_NULL_HANDLE) {
    printf("LOG: Destroying instance\n");
    vkDestroyInstance(instance.handle, nullptr);
  }
}

// Declare the stages
namespace {
Status SetupInstance(VulkanContext*);
Status SetupDebugMessenger(VulkanContext*);


// Utils -----------------------------------------------------------------------
// Adds all the required extensions from SDL and beyond.
Status AddInstanceExtensions(SDL_Window*, VulkanContext*);
Status AddInstanceValidationLayers(VulkanContext*);
Status CheckRequiredLayers(const std::vector<const char*>&);
}  // namespace

#define RETURN_IF_ERROR(status, call) \
  status = (call);                    \
  if (!status.ok())                   \
    return status;

Status
InitVulkanContext(SDL_Window* window, VulkanContext* context) {
  Status status;
  // Instance.
  RETURN_IF_ERROR(status, AddInstanceExtensions(window, context));
  RETURN_IF_ERROR(status, AddInstanceValidationLayers(context));
  RETURN_IF_ERROR(status, SetupInstance(context));
  RETURN_IF_ERROR(status, SetupDebugMessenger(context));

  return Status::Ok();
}

// Stages implementation.
namespace {

Status SetupInstance(VulkanContext* context) {
  // Vulkan application info.
  VkApplicationInfo app_info = {};
  app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName   = "Vulkan Test";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName        = "Warhol";
  app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion = VK_API_VERSION_1_1;

  // The creation info.
  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount =
      (uint32_t)context->instance.extensions.size();
  create_info.ppEnabledExtensionNames = context->instance.extensions.data();

  Status res = CheckRequiredLayers(context->instance.validation_layers);
  if (!res.ok())
    return res;

  create_info.enabledLayerCount =
      (uint32_t)context->instance.validation_layers.size();
  create_info.ppEnabledLayerNames = context->instance.validation_layers.data();

  // Finally create the VkInstance.
  VkResult result =
      vkCreateInstance(&create_info, nullptr, &context->instance.handle);
  VK_RETURN_IF_ERROR(result);
  return Status::Ok();
}

Status SetupDebugMessenger(VulkanContext* context) {
  VkDebugUtilsMessengerCreateInfoEXT messenger_info = {};
  messenger_info.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messenger_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  messenger_info.pfnUserCallback = VulkanDebugCall;
  messenger_info.pUserData = nullptr;

  Status status =
      CreateDebugUtilsMessengerEXT(context->instance.handle, &messenger_info, nullptr,
                                   &context->debug_messenger);
  return status;
}


// Utils -----------------------------------------------------------------------

Status
AddInstanceExtensions(SDL_Window* window, VulkanContext* context) {
  VK_GET_PROPERTIES(SDL_Vulkan_GetInstanceExtensions, window,
                    (context->instance.extensions));
  if (context->instance.extensions.empty())
    return Status("Could not get SDL required extensions");
  // Add the debug extensions.
  context->instance.extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  return Status::Ok();
}

Status
AddInstanceValidationLayers(VulkanContext* context) {
  // Add validation layers
  context->instance.validation_layers.push_back(
      "VK_LAYER_LUNARG_standard_validation");
  return Status::Ok();
}

Status
CheckRequiredLayers(const std::vector<const char*>& requested_layers) {
  if (requested_layers.empty())
    return Status::Ok();

  // Check available validation layers.
  std::vector<VkLayerProperties> available_layers;
  VK_GET_PROPERTIES_NC(vkEnumerateInstanceLayerProperties, available_layers);

  if (available_layers.empty())
    return Status("No layers available");

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
      return Status("Layer %s not found", requested_layer);
  }

  return Status::Ok();
};

}  // namespace





}  // namespace warhol
