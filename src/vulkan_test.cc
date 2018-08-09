// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>
#include <windows.h>

#include <memory>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL2/sdl.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "status.h"
#include "vulkan_utils.h"

#ifdef NDEBUG
bool kValidationLayersEnabled = false;
#else
bool kValidationLayersEnabled = true;
#endif

using namespace warhol;

Status
GetVulkanExtensions(SDL_Window* window, std::vector<const char*>* extensions) {
  // Setup extensions.
  // Get the ones needed by SDL
  uint32_t extension_count;
  if(!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, NULL)) {
    return Status("Error getting vulkan extensions: %s\n", SDL_GetError());
  }
  *extensions = std::vector<const char*>(extension_count);
  SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extensions->data());

  // Extra ones
  if (kValidationLayersEnabled) {
    extensions->push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return Status::Ok();
}

// Checks that the requested validation layers are present in the ones offered
// by our Vulkan runtime.
bool
CheckVulkanValidationLayers(const std::vector<const char *> &requested_layers) {
  // Check available validation layers.
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

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

Status
SetupVulkanInstance(SDL_Window* window, VulkanContext* context) {
  // Vulkan application info.
  VkApplicationInfo app_info  = {};
  app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName   = "Vulkan Test";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName        = "Warhol";
  app_info.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion         = VK_API_VERSION_1_1;

  std::vector<const char*> extensions;
  Status res = GetVulkanExtensions(window, &extensions);
  if (!res.ok()) return res;

  VkInstanceCreateInfo create_info    = {};
  create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo        = &app_info;
  create_info.enabledExtensionCount   = (uint32_t)extensions.size();
  create_info.ppEnabledExtensionNames = extensions.data();
  create_info.enabledLayerCount       = 0;

  // Setup validation layers.
  std::vector<const char*> validation_layers;
  if (kValidationLayersEnabled) {
    validation_layers.push_back("VK_LAYER_LUNARG_standard_validation");
    if (!CheckVulkanValidationLayers(validation_layers))
      return Status("Not all requested validation layers are available");

    create_info.enabledLayerCount   = (uint32_t)validation_layers.size();
    create_info.ppEnabledLayerNames = validation_layers.data();
  }

  // Finally create the VkInstance.
  VkResult result = vkCreateInstance(&create_info, nullptr, &context->instance);
  VK_RETURN_IF_ERROR(result);

  // Setup debug messenger.
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
  messenger_info.pUserData       = nullptr;

  Status status =
      CreateDebugUtilsMessengerEXT(context->instance, &messenger_info, nullptr,
                                   &context->debug_messenger_handle);
  if (!status.ok()) return status;

  return Status::Ok();
}

Status SetupVulkanDevices(VulkanContext* context) {
  // Check how many devices are present.
  uint32_t device_count = 0;
  VkResult res = vkEnumeratePhysicalDevices(context->instance, &device_count,
                                            nullptr);
  VK_RETURN_IF_ERROR(res);
  std::vector<VkPhysicalDevice> devices(device_count);
  res = vkEnumeratePhysicalDevices(context->instance, &device_count, devices.data());
  VK_RETURN_IF_ERROR(res);

  // Enumarate device properties.
  printf("Found %zu physical devices:\n", devices.size());
  for (auto& device : devices) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    printf("--------------------------------------------\n");
    printf("Device Name: %s\n", properties.deviceName);
    printf("Type: %d\n", properties.deviceType);
    printf("API Version: %u\n", properties.apiVersion);
    printf("Driver Version: %u\n", properties.driverVersion);
    printf("Vendor ID: %u\n", properties.vendorID);
    printf("Device ID: %u\n", properties.deviceID);
  }

  return Status::Ok();
}

int main() {
  // Setup SDL2.
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    printf("Error loading SDL: %s\n", SDL_GetError());
    return 1;
  }

  // Data about displays.
  printf("Information from SDL\n");
  printf("Amount of displays: %d\n", SDL_GetNumVideoDisplays());

  SDL_Window *window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, 640, 480,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
  if (!window) {
    printf("Error creating window: %s\n", SDL_GetError());
    return 1;
  }

  {
    VulkanContext context;
    Status res = SetupVulkanInstance(window, &context);
    if (!res.ok()) {
      printf("Error setting vulkan instance: %s\n", res.err_msg().c_str());
      return 1;
    }
    res = SetupVulkanDevices(&context);
    if (!res.ok()) {
      printf("Error setting vulkan devices: %s\n", res.err_msg().c_str());
      return 1;
    }

  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

