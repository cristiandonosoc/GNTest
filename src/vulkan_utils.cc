// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>
#include <string.h>

#include <SDL2/SDL_vulkan.h>

#include "macros.h"
#include "vulkan_context.h"
#include "vulkan_utils.h"

namespace warhol {

namespace {

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

}  // namespace

// VkInstance ------------------------------------------------------------------

Status
SetupSDLVulkanInstance(InstanceContext* instance) {
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
  create_info.enabledExtensionCount = (uint32_t)instance->extensions.size();
  create_info.ppEnabledExtensionNames = instance->extensions.data();

  if (!CheckRequiredLayers(instance->validation_layers))
    return Status("Not all requested validation layers are available");

  create_info.enabledLayerCount = (uint32_t)instance->validation_layers.size();
  create_info.ppEnabledLayerNames = instance->validation_layers.data();

  // Finally create the VkInstance.
  VkResult result = vkCreateInstance(&create_info, nullptr, &instance->handle);
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
  messenger_info.pUserData = nullptr;

  Status status =
      CreateDebugUtilsMessengerEXT(instance->handle, &messenger_info, nullptr,
                                   &instance->debug_messenger_handle);
  return status;
}

// Logical Device --------------------------------------------------------------

Status
SetupVulkanPhysicalDevices(InstanceContext* instance) {
  std::vector<VkPhysicalDevice> devices;
  VK_GET_PROPERTIES(vkEnumeratePhysicalDevices, instance->handle, devices);

  // Enumarate device properties.
  printf("Found %zu physical devices:\n", devices.size());
  for (auto& device : devices) {
    auto pd_context = std::make_unique<PhysicalDeviceContext>();
    pd_context->handle = device;
    vkGetPhysicalDeviceProperties(device, &pd_context->properties);
    vkGetPhysicalDeviceFeatures(device, &pd_context->features);

    printf("--------------------------------------------\n");
    printf("Device Name: %s\n", pd_context->properties.deviceName);
    printf("Type: %s\n", VulkanEnumToString(pd_context->properties.deviceType));
    printf("API Version: %u\n", pd_context->properties.apiVersion);
    printf("Driver Version: %u\n", pd_context->properties.driverVersion);
    printf("Vendor ID: %x\n", pd_context->properties.vendorID);
    printf("Device ID: %x\n", pd_context->properties.deviceID);

    // We setup the queue families data for each device.
    VK_GET_PROPERTIES(vkGetPhysicalDeviceQueueFamilyProperties, device,
                      (pd_context->qf_properties));
    instance->physical_devices.push_back(std::move(pd_context));
  }

  if (instance->physical_devices.empty())
    return Status("No suitable device found");
  return Status::Ok();
}

// Logical Device --------------------------------------------------------------

Status
SetupVulkanLogicalDevices(InstanceContext* instance) {
  auto device = std::make_unique<LogicalDeviceContext>();
  auto& physical_device = instance->physical_devices.back();
  // For now we get the graphical queue.
  int i = 0;
  for (auto& qfp : physical_device->qf_properties) {
    if (qfp.queueCount == 0)
      continue;

    // Get the graphical queue.
    if (qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      device->graphics_queue_index = i;

    // Get the present queue.
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device->handle, i,
                                         instance->surface, &present_support);
    if (present_support)
      device->present_queue_index = i;

    i++;
  }

  if (device->graphics_queue_index < 0)
    return Status("Could not find a graphical queue");
  if (device->present_queue_index < 0)
    return Status("Could not find a present queue");

  // The device ->ueues to set.
  float queue_priority = 1.0f;
  VkDeviceQueueCreateInfo qcreate_infos[2] = {};

  // Setup the graphics queue info.
  qcreate_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  qcreate_infos[0].queueFamilyIndex = device->graphics_queue_index;
  qcreate_infos[0].queueCount = 1;
  qcreate_infos[0].pQueuePriorities = &queue_priority;
  qcreate_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  qcreate_infos[1].queueFamilyIndex = device->graphics_queue_index;
  qcreate_infos[1].queueCount = 1;
  qcreate_infos[1].pQueuePriorities = &queue_priority;

  // Setup the present queue info.

  // Setup the logical device features.
  VkDeviceCreateInfo dci = {};
  dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  dci.queueCreateInfoCount = 2;
  dci.pQueueCreateInfos = qcreate_infos;
  // Set the enabled features.
  /* dci.pEnabledFeatures = &physical_device->features; */
  // For now physical features are disabled.
  VkPhysicalDeviceFeatures features = {};
  dci.pEnabledFeatures = &features;
  // Setup the validation layers.
  dci.enabledLayerCount = (uint32_t)instance->validation_layers.size();
  dci.ppEnabledLayerNames = instance->validation_layers.data();

  VkResult res = vkCreateDevice(physical_device->handle, &dci, nullptr,
                                &device->handle);
  VK_RETURN_IF_ERROR(res);

  // Get the graphics queue
  vkGetDeviceQueue(device->handle, device->graphics_queue_index, 0,
                   &device->graphics_queue);

  physical_device->logical_devices.push_back(std::move(device));

  return Status::Ok();
}

// Validation Layers -----------------------------------------------------------

Status
GetSDLExtensions(SDL_Window* window, InstanceContext* instance) {
  VK_GET_PROPERTIES(SDL_Vulkan_GetInstanceExtensions, window,
                    (instance->extensions));
  if (instance->extensions.empty())
    return Status("Could not get SDL required extensions");

  for (const char* ext : instance->extensions)
    printf("EXTENSION: %s\n", ext);
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
