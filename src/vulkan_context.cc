// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <string.h>

#include "vulkan_context.h"
#include "vulkan_utils.h"
namespace warhol {

// Declare the stages
namespace {
Status SetupSDLInstance(VulkanContext*);
Status SetupDebugMessenger(VulkanContext*);


// Utils.
bool CheckRequiredLayers(const std::vector<const char*>&);
}  // namespace

#define RETURN_IF_ERROR(status, call) \
  status = (call);                    \
  if (!status.ok())                   \
    return status;

Status
InitVulkanContext(VulkanContext* context) {
  Status status;
  RETURN_IF_ERROR(status, SetupSDLInstance(context));
  /* RETURN_IF_ERROR(status, SetupDebugMessenger(context)); */
  return Status::Ok();
}

// Stages implementation.
namespace {

Status SetupSDLInstance(VulkanContext* context) {
  // Vulkan application info.
  VkApplicationInfo app_info = {};
  app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName   = "Vulkan Test";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName        = "Warhol";
  app_info.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion         = VK_API_VERSION_1_1;

  // The creation info.
  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = (uint32_t)context->instance.extensions.size();
  create_info.ppEnabledExtensionNames = context->instance.extensions.data();

  if (!CheckRequiredLayers(context->instance.validation_layers))
    return Status("Not all requested validation layers are available");

  create_info.enabledLayerCount = (uint32_t)context->instance.validation_layers.size();
  create_info.ppEnabledLayerNames = context->instance.validation_layers.data();

  // Finally create the VkInstance.
  VkResult result = vkCreateInstance(&create_info, nullptr, &context->instance.handle);
  VK_RETURN_IF_ERROR(result);
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





}  // namespace




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

  if (debug_messenger != VK_NULL_HANDLE)
    DestroyDebugUtilsMessengerEXT(instance.handle, debug_messenger, nullptr);

  if (instance.handle != VK_NULL_HANDLE)
    vkDestroyInstance(instance.handle, nullptr);
}




}  // namespace warhol
