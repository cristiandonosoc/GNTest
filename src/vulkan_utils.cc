// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>
#include <string.h>

#include <limits>

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

// Physical Device -------------------------------------------------------------

namespace {

SwapChainProperties
GetSwapChainProperties(const PhysicalDeviceContext& physical_device) {
  // Setup the swap chain.
  SwapChainProperties properties;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physical_device.handle, physical_device.surface, &properties.capabilites);
  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(
      physical_device.handle, physical_device.surface, &format_count, nullptr);
  properties.formats.resize(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device.handle,
                                       physical_device.surface, &format_count,
                                       properties.formats.data());

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device.handle,
                                            physical_device.surface,
                                            &present_mode_count, nullptr);

  properties.present_modes.resize(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device.handle, physical_device.surface, &present_mode_count,
      properties.present_modes.data());

  return properties;
}

}  // namespace

Status
SetupVulkanPhysicalDevices(SDL_Window* window, InstanceContext* instance) {
  std::vector<VkPhysicalDevice> devices;
  VK_GET_PROPERTIES(vkEnumeratePhysicalDevices, instance->handle, devices);

  // Enumarate device properties.
  printf("Found %zu physical devices:\n", devices.size());
  for (auto& device : devices) {
    auto physical_device = std::make_unique<PhysicalDeviceContext>(instance);
    physical_device->handle = device;
    vkGetPhysicalDeviceProperties(device, &physical_device->properties);
    vkGetPhysicalDeviceFeatures(device, &physical_device->features);

    printf("--------------------------------------------\n");
    printf("Device Name: %s\n", physical_device->properties.deviceName);
    printf("Type: %s\n",
           VulkanEnumToString(physical_device->properties.deviceType));
    printf("API Version: %u\n", physical_device->properties.apiVersion);
    printf("Driver Version: %u\n", physical_device->properties.driverVersion);
    printf("Vendor ID: %x\n", physical_device->properties.vendorID);
    printf("Device ID: %x\n", physical_device->properties.deviceID);
    fflush(stdout);

    // We setup the queue families data for each device.
    VK_GET_PROPERTIES(vkGetPhysicalDeviceQueueFamilyProperties, device,
                      (physical_device->qf_properties));

    // Create the surface associated with this device.
    Status res = CreateSurface(window, instance, physical_device.get());
    if (!res.ok())
      return res;

    // Get the queues
    int i = 0;
    for (auto& qfp : physical_device->qf_properties) {
      if (qfp.queueCount == 0)
        continue;

      // Get the graphical queue.
      if (qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        physical_device->graphics_queue_index = i;

      // Get the present queue.
      VkBool32 present_support = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(physical_device->handle, i,
                                           physical_device->surface,
                                           &present_support);
      if (present_support)
        physical_device->present_queue_index = i;

      i++;
    }

    physical_device->swap_chain_properties =
        GetSwapChainProperties(*physical_device.get());

    instance->physical_devices.push_back(std::move(physical_device));
  }

  if (instance->physical_devices.empty())
    return Status("No suitable device found");
  return Status::Ok();
}

Status
CreateSurface(SDL_Window* window, InstanceContext* instance,
              PhysicalDeviceContext* physical_device) {
  assert(physical_device->instance);
  if (!SDL_Vulkan_CreateSurface(window, instance->handle,
                                &physical_device->surface)) {
    return Status("Could not create surface: %s\n", SDL_GetError());
  }
  return Status::Ok();
}

namespace {

// A suitable physical device has the following properties:
// - Both graphics and present queues.
// - All the required extensions.
// - At least one swap chain image format and present mode.
bool
IsSuitablePhysicalDevice(const PhysicalDeviceContext& physical_device,
                         const std::vector<const char*>& requested_extensions) {
  // Queues.
  if (physical_device.graphics_queue_index < 0 ||
      physical_device.present_queue_index < 0)
    return false;

  // Extensions.
  if (!CheckPhysicalDeviceRequiredExtensions(physical_device,
                                             requested_extensions)) {
    return false;
  }

  // Swap chain properties.
  if (physical_device.swap_chain_properties.formats.empty() ||
      physical_device.swap_chain_properties.present_modes.empty()) {
    return false;
  }

  return true;
}



}  // namespace

PhysicalDeviceContext*
FindSuitablePhysicalDevice(
    InstanceContext* instance,
    const std::vector<const char*>& requested_extensions) {
  // We check for valid physical devices.
  // For now we pick the first one.
  PhysicalDeviceContext* physical_device = nullptr;
  for (const auto& pd : instance->physical_devices) {
    if (IsSuitablePhysicalDevice(*pd, requested_extensions)) {
      physical_device = pd.get();
      break;
    }
  }
  return physical_device;
}

// Logical Device --------------------------------------------------------------

Status
SetupVulkanLogicalDevices(
    InstanceContext* instance,
    PhysicalDeviceContext* physical_device,
    const std::vector<const char*>& requested_extensions) {

  // The device ->ueues to set.
  float queue_priority = 1.0f;
  VkDeviceQueueCreateInfo qcreate_infos[2];

  // Setup the graphics queue.
  qcreate_infos[0] = {};
  qcreate_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  qcreate_infos[0].queueFamilyIndex = physical_device->graphics_queue_index;
  qcreate_infos[0].queueCount = 1;
  qcreate_infos[0].pQueuePriorities = &queue_priority;
  // Setup the present queue.
  qcreate_infos[1] = {};
  qcreate_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  qcreate_infos[1].queueFamilyIndex = physical_device->present_queue_index;
  qcreate_infos[1].queueCount = 1;
  qcreate_infos[1].pQueuePriorities = &queue_priority;

  // Setup the logical device features.
  VkDeviceCreateInfo dci = {};
  dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  dci.queueCreateInfoCount = 2;
  dci.pQueueCreateInfos = qcreate_infos;
  // Extensions.
  dci.enabledExtensionCount = (uint32_t)requested_extensions.size();
  dci.ppEnabledExtensionNames = requested_extensions.data();
  // Features.
  // For now physical features are disabled.
  /* dci.pEnabledFeatures = &physical_device->features; */
  VkPhysicalDeviceFeatures features = {};
  dci.pEnabledFeatures = &features;
  // Validation layers.
  dci.enabledLayerCount = (uint32_t)instance->validation_layers.size();
  dci.ppEnabledLayerNames = instance->validation_layers.data();

  // We now setup the device
  auto device = std::make_unique<LogicalDeviceContext>(physical_device);

  // Finally create the device.
  VkResult res = vkCreateDevice(physical_device->handle, &dci, nullptr,
                                &device->handle);
  VK_RETURN_IF_ERROR(res);

  // Get the graphics queue.
  vkGetDeviceQueue(device->handle, physical_device->graphics_queue_index, 0,
                   &device->graphics_queue);
  // Get the present queue.
  vkGetDeviceQueue(device->handle, physical_device->present_queue_index, 0,
                   &device->present_queue);

  physical_device->logical_devices.push_back(std::move(device));
  physical_device->selected_logical_device =
      physical_device->logical_devices.back().get();

  return Status::Ok();
}

// SwapChain -------------------------------------------------------------------

namespace {

VkSurfaceFormatKHR
GetBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
  // When undefined, vulkan lets us decided whichever we want.
  if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
    return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

  // We try to find the RGBA SRGB format.
  for (const auto& format : formats) {
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
        format.colorSpace ==  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }

  // Otherwise return the first one.
  return formats.front();
}

static inline bool
HasPresentMode(const std::vector<VkPresentModeKHR>& present_modes,
               const VkPresentModeKHR& target) {
  for (const auto& present_mode : present_modes) {
    if (present_mode == target)
      return true;
  }
  return false;
}

VkPresentModeKHR
GetBestPresentMode(const std::vector<VkPresentModeKHR>& present_modes) {
  if (HasPresentMode(present_modes, VK_PRESENT_MODE_MAILBOX_KHR))
      return VK_PRESENT_MODE_MAILBOX_KHR;

  // Sometimes FIFO is not well implemented, so prefer immediate.
  if (HasPresentMode(present_modes, VK_PRESENT_MODE_IMMEDIATE_KHR))
    return VK_PRESENT_MODE_IMMEDIATE_KHR;

  // FIFO is assured to be there.
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    return capabilities.maxImageExtent;

#if 0
    // Do the clamping.
    VkExtent2D extent = {WIDTH, HEIGHT};
    // TODO: Use MAX, MIN macros.
    extent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, extent.width));
    extent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, extent.height));

    return extent;
#endif
  }
}

}  // namespace


#if 0
Status
SetupSwapChain(InstanceContext* instance) {
  auto* physical_device = instance->selected_physical_device;
  if (physical_device)
    return Status("No physical device selected");

  SwapChainContext* swap_chain = physical_device->swap_chain.get();
  if (!swap_chain)
    return Status("No Swap Chain context generated");

  uint32_t image_min = swap_chain->capabilites.minImageCount;
  uint32_t image_max = swap_chain->capabilites.maxImageCount;
  uint32_t image_count = image_min + 1;
  if (image_max > 0 && image_count > image_max)
    image_count = image_max;

  auto surface_format = GetBestSurfaceFormat(swap_chain->formats);
  auto present_mode = GetBestPresentMode(swap_chain->present_modes);
  auto extent = ChooseSwapExtent(swap_chain->capabilites);


  VkSwapchainCreateInfoKHR scci = {};
  scci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  scci.surface = instance->surface;

  scci.minImageCount = image_count;
  scci.imageFormat = surface_format.format;
  scci.imageColorSpace = surface_format.colorSpace;
  scci.imageExtent = extent;
  scci.imageArrayLayers = 1;
  scci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queue_family_indices[] = {
      (uint32_t)physical_device->graphics_queue_index,
      (uint32_t)physical_device->present_queue_index};

  // We check which sharing mode is needed between the command queues.
  if (physical_device->graphics_queue_index !=
      physical_device->present_queue_index) {
    // If graphics and present are different, we have a concurrent management
    // that is simpler.
    scci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    scci.queueFamilyIndexCount = 2;
    scci.pQueueFamilyIndices = queue_family_indices;
  } else {
    // If the queues are the same, we use exclusive as there are no queues
    // to coordinate.
    scci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    scci.queueFamilyIndexCount = 0;      // Optional
    scci.pQueueFamilyIndices = nullptr;  // Optional
  }

  scci.preTransform = swap_chain->capabilites.currentTransform;
  // Don't blend alpha between images of the swap chain.
  scci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  scci.presentMode = present_mode;
  scci.clipped = VK_TRUE;   // Ignore pixels that are ignored.

  // If we need to create a new swap chain, this is a reference to the one we
  // came from.
  scci.oldSwapchain = VK_NULL_HANDLE;







  return Status::Ok();
}

#endif

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
CheckPhysicalDeviceRequiredExtensions(
    const PhysicalDeviceContext& physical_device,
    const std::vector<const char*>& requested_extensions) {
  if (requested_extensions.empty())
    return true;

  // Get the the extensions the physical device actually offers.
  uint32_t extension_count = 0;
  vkEnumerateDeviceExtensionProperties(physical_device.handle,
                                       nullptr,
                                       &extension_count,
                                       nullptr);
  std::vector<VkExtensionProperties> available_extensions;
  available_extensions.resize(extension_count);
  vkEnumerateDeviceExtensionProperties(physical_device.handle,
                                       nullptr,
                                       &extension_count,
                                       available_extensions.data());

  // All extensions should be present.
  for (const char* requested_extension : requested_extensions) {
    bool found = false;
    for (const auto& available_extension : available_extensions) {
      if (strcmp(available_extension.extensionName,
                 requested_extension) == 0) {
        found = true;
        break;
      }
    }

    if (!found)
      return false;
  }

  return true;
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
