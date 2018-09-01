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

  if (physical_device.swap_chain.handle != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(logical_device.handle,
                          physical_device.swap_chain.handle, nullptr);

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
Status SetupInstance(SDL_Window*, VulkanContext*);
Status SetupDebugMessenger(VulkanContext*);
Status SetupPhysicalDevice(VulkanContext*);
Status SetupSurface(SDL_Window*, VulkanContext*);

// Utils -----------------------------------------------------------------------
// Adds all the required extensions from SDL and beyond.
Status AddInstanceExtensions(SDL_Window*, VulkanContext*);
Status AddInstanceValidationLayers(VulkanContext*);
Status CheckRequiredLayers(const std::vector<const char*>&);

VulkanContext::PhysicalDevice::SwapChain
GetSwapChainProperties(const VulkanContext&,
                       const VulkanContext::PhysicalDevice&);
bool
IsSuitablePhysicalDevice(const VulkanContext::PhysicalDevice&,
                         const std::vector<const char*>& extensions);

}  // namespace

#define RETURN_IF_ERROR(status, call) \
  status = (call);                    \
  if (!status.ok())                   \
    return status;

Status
InitVulkanContext(SDL_Window* window, VulkanContext* context) {
  Status status;
  // Instance.
  RETURN_IF_ERROR(status, SetupInstance(window, context));
  RETURN_IF_ERROR(status, SetupDebugMessenger(context));
  RETURN_IF_ERROR(status, SetupSurface(window, context));
  RETURN_IF_ERROR(status, SetupPhysicalDevice(context));

  return Status::Ok();
}

// Stages implementation.
namespace {

Status SetupInstance(SDL_Window* window, VulkanContext* context) {
  // TODO: The extensions and validation layers should be exposed.
  Status status;
  RETURN_IF_ERROR(status, AddInstanceExtensions(window, context));
  RETURN_IF_ERROR(status, AddInstanceValidationLayers(context));

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

Status
SetupDebugMessenger(VulkanContext* context) {
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
      CreateDebugUtilsMessengerEXT(context->instance.handle, &messenger_info,
                                   nullptr, &context->debug_messenger);
  return status;
}

Status
SetupSurface(SDL_Window* window, VulkanContext* context) {
  if (!SDL_Vulkan_CreateSurface(window, context->instance.handle,
                                &context->surface)) {
    return Status("Could not create surface: %s\n", SDL_GetError());
  }
  return Status::Ok();
}

Status
SetupPhysicalDevice(VulkanContext* context) {
  std::vector<VkPhysicalDevice> device_handles;
  VK_GET_PROPERTIES(vkEnumeratePhysicalDevices, context->instance.handle,
                    device_handles);

  // Enumarate device properties.
  std::vector<VulkanContext::PhysicalDevice> devices;
  devices.reserve(device_handles.size());
  printf("Found %zu physical devices:\n", device_handles.size());
  for (auto& device_handle : device_handles) {
    VulkanContext::PhysicalDevice device;
    device.handle = device_handle;
    vkGetPhysicalDeviceProperties(device.handle, &device.properties);
    vkGetPhysicalDeviceFeatures(device.handle, &device.features);

    printf("--------------------------------------------\n");
    printf("Device Name: %s\n", device.properties.deviceName);
    printf("Type: %s\n", VulkanEnumToString(device.properties.deviceType));
    printf("API Version: %u\n", device.properties.apiVersion);
    printf("Driver Version: %u\n", device.properties.driverVersion);
    printf("Vendor ID: %x\n", device.properties.vendorID);
    printf("Device ID: %x\n", device.properties.deviceID);
    fflush(stdout);

    // We setup the queue families data for each device.
    VK_GET_PROPERTIES(vkGetPhysicalDeviceQueueFamilyProperties, device.handle,
                      (device.qf_properties));

    // Get the queues
    int i = 0;
    for (auto& qfp : device.qf_properties) {
      if (qfp.queueCount == 0)
        continue;

      // Get the graphical queue.
      if (qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        device.graphics_queue_index = i;

      // Get the present queue.
      VkBool32 present_support = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device.handle, i, context->surface,
                                           &present_support);
      if (present_support)
        device.present_queue_index = i;

      i++;
    }

    device.swap_chain = GetSwapChainProperties(*context, device);

    // We check if this is a suitable device
    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (IsSuitablePhysicalDevice(device, extensions)) {
      printf("Device \"%s\" is not suitable\n", device.properties.deviceName);
      devices.push_back(std::move(device));
    }
  }

  if (devices.empty())
    return Status("No suitable device found");

  // TODO: Find a better heuristic to get the device.
  //       For now we get the first.
  context->physical_device = devices.front();
  printf("Selected device \"%s\"\n",
         context->physical_device.properties.deviceName);

  return Status::Ok();
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

VulkanContext::PhysicalDevice::SwapChain
GetSwapChainProperties(const VulkanContext& context,
                       const VulkanContext::PhysicalDevice& device) {
  // Setup the swap chain.
  VulkanContext::PhysicalDevice::SwapChain swap_chain;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.handle, context.surface,
                                            &swap_chain.capabilites);
  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device.handle, context.surface,
                                       &format_count, nullptr);
  swap_chain.formats.resize(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(
      device.handle, context.surface, &format_count, swap_chain.formats.data());

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device.handle, context.surface,
                                            &present_mode_count, nullptr);

  swap_chain.present_modes.resize(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device.handle, context.surface,
                                            &present_mode_count,
                                            swap_chain.present_modes.data());

  return swap_chain;
}



bool
IsSuitablePhysicalDevice(const VulkanContext::PhysicalDevice& device,
                         const std::vector<const char*>& extensions) {
  // Queues.
  if (device.graphics_queue_index < 0 ||
      device.present_queue_index < 0)
    return false;


  if (extensions.empty())
    return true;

  // Get the the extensions the physical device actually offers.
  uint32_t extension_count = 0;
  vkEnumerateDeviceExtensionProperties(device.handle,
                                       nullptr,
                                       &extension_count,
                                       nullptr);
  std::vector<VkExtensionProperties> available_extensions;
  available_extensions.resize(extension_count);
  vkEnumerateDeviceExtensionProperties(device.handle,
                                       nullptr,
                                       &extension_count,
                                       available_extensions.data());

  // All extensions should be present.
  for (const char* extension : extensions) {
    bool found = false;
    for (const auto& available_extension : available_extensions) {
      if (strcmp(available_extension.extensionName,
                 extension) == 0) {
        found = true;
        break;
      }
    }

    if (!found)
      return false;
  }

  // Swap chain properties.
  if (device.swap_chain.formats.empty() ||
      device.swap_chain.present_modes.empty()) {
    return false;
  }

  return true;
}

}  // namespace

}  // namespace warhol
