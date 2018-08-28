// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>
#include <string.h>

#include <limits>
#include <set>

#include <SDL2/SDL_vulkan.h>

#include "vulkan_context.h"
#include "vulkan_utils.h"

namespace warhol {

// InstanceContext ------------------------------------------------------------

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

InstanceContext::InstanceContext() = default;
InstanceContext::~InstanceContext() {
  // We need to free the children first, as they have a reference to the vulkan
  // instance within this context.
  physical_devices.clear();

  if (handle == VK_NULL_HANDLE)
    return;

  // IMPORTANT: Keep the correct dependency order of destruction.
  if (debug_messenger_handle != VK_NULL_HANDLE)
    DestroyDebugUtilsMessengerEXT(handle, debug_messenger_handle, nullptr);


  vkDestroyInstance(handle, nullptr);
}

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

// PhysicalDeviceContext ------------------------------------------------------

PhysicalDeviceContext::PhysicalDeviceContext(InstanceContext* instance)
    : instance(instance) {}
PhysicalDeviceContext::~PhysicalDeviceContext() {
  assert(instance);

  // Clear the logical devices first.
  logical_devices.clear();

  if (surface != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(instance->handle, surface, nullptr);
}

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

// LogicalDeviceContext -------------------------------------------------------

LogicalDeviceContext::LogicalDeviceContext(
    PhysicalDeviceContext* physical_device)
    : physical_device(physical_device) {}
LogicalDeviceContext::~LogicalDeviceContext() {
  // Clear the swap chains.
  swap_chain.reset();

  if (handle != VK_NULL_HANDLE)
    vkDestroyDevice(handle, nullptr);
}

Status
SetupVulkanLogicalDevices(
    InstanceContext* instance,
    PhysicalDeviceContext* physical_device,
    const std::vector<const char*>& requested_extensions) {

  // The device queues to set.
  float queue_priority = 1.0f;

  // We onlu create unique device queues
  std::set<int> queue_indices = {physical_device->graphics_queue_index,
                                 physical_device->present_queue_index};
  std::vector<VkDeviceQueueCreateInfo> qcreate_infos;
  for (int queue_family : queue_indices) {
    VkDeviceQueueCreateInfo qcreate_info = {};
    qcreate_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qcreate_info.queueFamilyIndex = queue_family;
    qcreate_info.queueCount = 1;
    qcreate_info.pQueuePriorities = &queue_priority;
    qcreate_infos.push_back(std::move(qcreate_info));
  }

  // Setup the logical device features.
  VkDeviceCreateInfo dci = {};
  dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  dci.queueCreateInfoCount = (uint32_t)qcreate_infos.size();
  dci.pQueueCreateInfos = qcreate_infos.data();
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

  return Status::Ok();
}

// SwapChainContext -----------------------------------------------------------

SwapChainContext::SwapChainContext(LogicalDeviceContext* logical_device)
    : logical_device(logical_device) {}
SwapChainContext::~SwapChainContext() {
  assert(logical_device);

  // Destroy the image views first.
  image_views.clear();

  if (handle != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(logical_device->handle, handle, nullptr);
}

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

Status
SetupSwapChain(PhysicalDeviceContext* physical_device,
               LogicalDeviceContext* logical_device) {
  if (!physical_device)
    return Status("No physical device selected");

  const SwapChainProperties& swap_chain_prop =
      physical_device->swap_chain_properties;

  uint32_t image_min = swap_chain_prop.capabilites.minImageCount;
  uint32_t image_max = swap_chain_prop.capabilites.maxImageCount;
  uint32_t image_count = image_min + 1;
  if (image_max > 0 && image_count > image_max)
    image_count = image_max;

  auto swap_chain = std::make_unique<SwapChainContext>(logical_device);
  swap_chain->surface_format = GetBestSurfaceFormat(swap_chain_prop.formats);
  swap_chain->present_mode = GetBestPresentMode(swap_chain_prop.present_modes);
  swap_chain->extent = ChooseSwapExtent(swap_chain_prop.capabilites);

  VkSwapchainCreateInfoKHR scci = {};
  scci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  scci.surface = physical_device->surface;

  scci.minImageCount = image_count;
  scci.imageFormat = swap_chain->surface_format.format;
  scci.imageColorSpace = swap_chain->surface_format.colorSpace;
  scci.imageExtent = swap_chain->extent;
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

  scci.preTransform = swap_chain_prop.capabilites.currentTransform;
  // Don't blend alpha between images of the swap chain.
  scci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  scci.presentMode = swap_chain->present_mode;
  scci.clipped = VK_TRUE;   // Ignore pixels that are ignored.

  // If we need to create a new swap chain, this is a reference to the one we
  // came from.
  scci.oldSwapchain = VK_NULL_HANDLE;

  VkResult res = vkCreateSwapchainKHR(logical_device->handle, &scci, nullptr,
                                      &swap_chain->handle);
  if (res != VK_SUCCESS)
    return Status("Cannot create swap chain: %s", VulkanEnumToString(res));

  // Retrieve the images.
  uint32_t sc_image_count;
  vkGetSwapchainImagesKHR(logical_device->handle, swap_chain->handle,
                          &sc_image_count, nullptr);
  swap_chain->images.resize(sc_image_count);
  vkGetSwapchainImagesKHR(logical_device->handle, swap_chain->handle,
                          &sc_image_count, swap_chain->images.data());

  logical_device->swap_chain = std::move(swap_chain);
  return Status::Ok();
}

// ImageView -------------------------------------------------------------------

ImageViewContext::ImageViewContext(SwapChainContext* swap_chain)
    : swap_chain(swap_chain) {}
ImageViewContext::~ImageViewContext() {
  assert(swap_chain);

  if (handle != VK_NULL_HANDLE)
    vkDestroyImageView(swap_chain->logical_device->handle, handle, nullptr);
}
ImageViewContext::ImageViewContext(ImageViewContext&&) = default;
ImageViewContext&
ImageViewContext::operator=(ImageViewContext&&) = default;

Status
CreateImageViews(SwapChainContext* swap_chain) {
  swap_chain->image_views.reserve(swap_chain->images.size());
  for (size_t i = 0; i < swap_chain->images.size(); i++) {
    swap_chain->image_views.emplace_back(ImageViewContext(swap_chain));

    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = swap_chain->images[i];
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = swap_chain->surface_format.format;

    ivci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.layerCount = 1;

    VkResult res = vkCreateImageView(swap_chain->logical_device->handle,
                                     &ivci,
                                     nullptr,
                                     &swap_chain->image_views[i].handle);
    if (res != VK_SUCCESS)
      return Status("Could not create image view: %s", VulkanEnumToString(res));
  }

  return Status::Ok();
}


// Misc ------------------------------------------------------------------------

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



}  // namespace warhol
