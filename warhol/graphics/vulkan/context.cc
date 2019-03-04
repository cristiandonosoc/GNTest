// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/context.h"

#include <string.h>

#include <iostream>
#include <set>

#include "warhol/graphics/vulkan/image_utils.h"
#include "warhol/graphics/vulkan/utils.h"

#include "warhol/assets/assets.h"
#include "warhol/math/vec.h"
#include "warhol/utils/log.h"
#include "warhol/utils/types.h"
#include "warhol/window/window_manager.h"

namespace warhol {
namespace vulkan {

// InitVulkanContext -----------------------------------------------------------

namespace {

inline void Header(const char* header) {
  std::cout << "\n*** " << header
            << " **************************************************************"
               "**************************\n\n";
  std::flush(std::cout);
}

// TODO: Setup a better debug call.
static VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCall(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                VkDebugUtilsMessageTypeFlagsEXT type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                void* user_data) {
  (void)user_data;
  if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    return VK_FALSE;

  // Message is important enough to show
  std::cout << "*VULKAN* [" << vulkan::EnumToString(severity) << "]["
            << vulkan::EnumToString(type) << "] " << callback_data->pMessage
            << std::endl;
  std::flush(std::cout);

  return VK_FALSE;
}

}  // namespace

bool InitVulkanContext(Context* context, WindowManager* window) {
  ASSERT(window->valid());
  context->extensions = WindowManagerGetVulkanInstanceExtensions(window);
  if (context->extensions.empty())
    return false;

#ifndef NDEBUG
  vulkan::AddDebugExtensions(&context->extensions);
  context->validation_layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

  if (!vulkan::CheckExtensions(context->extensions))
    return false;

  if (!vulkan::CheckValidationLayers(context->validation_layers))
    return false;

  Header("Creating context...");
  if (!vulkan::CreateContext(context))
    return false;

  Header("Set debug callback....");
  if (!vulkan::SetupDebugCall(context, VulkanDebugCall))
    return false;

  Header("Creating surface...");
  VkSurfaceKHR surface;

  if (!WindowManagerCreateVulkanSurface(
          window, &context->instance.value(), &surface)) {
    return false;
  }
  context->surface.Set(context, surface);

  context->device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

  if (!vulkan::PickPhysicalDevice(context))
    return false;
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(context->physical_device, &properties);
  LOG(INFO) << "Picked physical device: " << properties.deviceName;

  Header("Creating a logical device...");
  if (!CreateLogicalDevice(context))
    return false;

  Header("Initializing resource management...");
  if (!InitResourceManagement(context))
    return false;

  Header("Creating a swap chain...");
  Pair<uint32_t> screen_size = {(uint32_t)window->width,
                                (uint32_t)window->height};
  if (!CreateSwapChain(context, screen_size))
    return false;

  Header("Creating image views...");
  if (!CreateImageViews(context))
    return false;

  Header("Creating a command pool for each framebuffer...");
  if (!CreateCommandPool(context))
    return false;

  Header("Creating depth buffers...");
  if (!CreateDepthResources(context))
    return false;
  return true;
}

// CreateContext ---------------------------------------------------------------

bool CreateContext(Context* context) {
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Warhol Vulkan Test";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName = "Warhol";
  app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  create_info.enabledExtensionCount = context->extensions.size();
  create_info.ppEnabledExtensionNames = context->extensions.data();

  create_info.enabledLayerCount = context->validation_layers.size();
  create_info.ppEnabledLayerNames = context->validation_layers.data();

  VkInstance instance;
  if (!VK_CALL(vkCreateInstance, &create_info, nullptr, &instance))
    return false;

  context->instance.Set(context, instance);
  return true;
}

// SetupDebugCall --------------------------------------------------------------

bool SetupDebugCall(Context* context,
                    PFN_vkDebugUtilsMessengerCallbackEXT callback) {
  VkDebugUtilsMessengerCreateInfoEXT create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

  create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pUserData = nullptr;
  create_info.pfnUserCallback = callback;

  VkDebugUtilsMessengerEXT debug_messenger;
  if (!CreateDebugUtilsMessengerEXT(
          context->instance.value(), &create_info, nullptr, &debug_messenger)) {
    return false;
  }

  context->debug_messenger.Set(context, debug_messenger);
  return true;
}

// PickPhysicalDevice ----------------------------------------------------------

namespace {

QueueFamilyIndices FindQueueFamilyIndices(const VkPhysicalDevice& device,
                                          const VkSurfaceKHR& surface) {
  QueueFamilyIndices indices = {};

  std::vector<VkQueueFamilyProperties> queue_families;
  VK_GET_PROPERTIES(vkGetPhysicalDeviceQueueFamilyProperties,
                    device,
                    queue_families);

  // We look for a GPU that has a graphics queue.
  for (size_t i = 0; i < queue_families.size(); i++) {
    auto& queue_family = queue_families[i];
    if (queue_family.queueCount == 0)
      continue;

    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      indices.graphics = i;

    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (present_support)
      indices.present = i;
  }

  return indices;
}

SwapChainCapabilities QuerySwapChainSupport(const VkPhysicalDevice& device,
                                            const VkSurfaceKHR& surface) {
  SwapChainCapabilities details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  VK_GET_PROPERTIES4(vkGetPhysicalDeviceSurfaceFormatsKHR,
                     device, surface, details.formats);

  LOG(INFO) << "  Got device properties: ";
  for (const auto& format : details.formats)
    LOG(INFO) << "  - Format: " << EnumToString(format.format)
               << ", Color Space: " << EnumToString(format.colorSpace);

  VK_GET_PROPERTIES4(vkGetPhysicalDeviceSurfacePresentModesKHR,
                     device, surface, details.present_modes);
  for (const auto& present_mode : details.present_modes)
    LOG(INFO) << "  - Present Mode: " << EnumToString(present_mode);

  return details;
}

PhysicalDeviceInfo GetPhysicalDeviceInfo(const VkPhysicalDevice& device,
                                         const VkSurfaceKHR& surface) {
  PhysicalDeviceInfo info;
  vkGetPhysicalDeviceProperties(device, &info.properties);
  vkGetPhysicalDeviceFeatures(device, &info.features);
  vkGetPhysicalDeviceMemoryProperties(device, &info.memory_properties);
  info.queue_family_indices = FindQueueFamilyIndices(device, surface);
  info.swap_chain_capabilities = QuerySwapChainSupport(device, surface);
  return info;
}

bool IsSuitableDevice(const VkPhysicalDevice& device,
                      const PhysicalDeviceInfo& device_info,
                      const std::vector<const char*>& required_extensions) {
  LOG(INFO) << "Checking GPU: " << device_info.properties.deviceName;

  auto& indices = device_info.queue_family_indices;
  if (indices.graphics == -1 || indices.present == -1)
    return false;

  if (!CheckPhysicalDeviceExtensions(device, required_extensions))
    return false;

  // We check for features.
  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(device, &features);

  if (!features.samplerAnisotropy) {
    LOG(ERROR) << "Require sampler anisotropy feature.";
    return false;
  }

  auto& capabilities = device_info.swap_chain_capabilities;
  if (capabilities.formats.empty() || capabilities.present_modes.empty())
    return false;
  return true;
}

}  // namespace

bool PickPhysicalDevice(Context* context) {
  std::vector<VkPhysicalDevice> devices;
  VK_GET_PROPERTIES(vkEnumeratePhysicalDevices, *context->instance, devices);
  if (devices.empty()) {
    LOG(ERROR) << "Could not find GPUs with Vulkan support.";
    return false;
  }

  std::vector<std::pair<VkPhysicalDevice, PhysicalDeviceInfo>> suitable_devices;
  for (const VkPhysicalDevice& device : devices) {
    PhysicalDeviceInfo device_info =
        GetPhysicalDeviceInfo(device, *context->surface);
    if (IsSuitableDevice(device, device_info, context->device_extensions)) {
      suitable_devices.push_back({device, device_info});
      LOG(INFO) << "- Suitable!";
    }
  }

  if (suitable_devices.empty()) {
    LOG(ERROR) << "Could not find a suitable physical device.";
    return false;
  }

  // For now we choose the first device.
  auto& [device, device_info] = suitable_devices.front();
  context->physical_device = device;
  context->physical_device_info = device_info;
  return true;
}

// CreateLogicalDevice ---------------------------------------------------------

bool CreateLogicalDevice(Context* context) {
  // Setup the queues.
  /* auto indices = FindQueueFamilyIndices(context->physical_device, */
  /*                                       *context->surface); */

  // We create one per queue family required. If there are the same, we only
  // create one.
  auto& indices = context->physical_device_info.queue_family_indices;
  int queue_indices[] = { indices.graphics, indices.present };
  VkDeviceQueueCreateInfo queue_create_infos[ARRAY_SIZE(queue_indices)] = {};

  std::set<int> unique_queues;
  unique_queues.insert(indices.graphics);
  unique_queues.insert(indices.present);

  float queue_priority = 1.0f;
  for (size_t i = 0; i < unique_queues.size(); i++) {
    auto& queue_create_info = queue_create_infos[i];
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_indices[i];
    queue_create_info.queueCount = 1; // Only need 1 queue of this kind.
    queue_create_info.pQueuePriorities = &queue_priority;
  }

  VkDeviceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount = unique_queues.size();
  create_info.pQueueCreateInfos = queue_create_infos;
  VkPhysicalDeviceFeatures enabled_features = {};
  enabled_features.samplerAnisotropy = VK_TRUE;
  create_info.pEnabledFeatures = &enabled_features;
  create_info.enabledExtensionCount = context->device_extensions.size();
  create_info.ppEnabledExtensionNames = context->device_extensions.data();

  VkDevice device;
  if (!VK_CALL(vkCreateDevice, context->physical_device, &create_info, nullptr,
                               &device)) {
    return false;
  }

  context->device.Set(context, device);
  vkGetDeviceQueue(device, indices.graphics, 0, &context->graphics_queue);
  vkGetDeviceQueue(device, indices.present, 0, &context->present_queue);

  return true;
}

// InitResourceManagement ------------------------------------------------------

bool InitResourceManagement(Context* context) {
  InitAllocator(context, &context->allocator, MEGABYTES(256), MEGABYTES(256));
  InitStagingManager(context, &context->staging_manager, MEGABYTES(128));

  return true;
}

// CreateSwapChain -------------------------------------------------------------

namespace {

VkSurfaceFormatKHR
ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
  // If vulkan only returns an undefined format, it means that it has no
  // preference and we can choose.
  if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
    VkSurfaceFormatKHR result;
    result.format = VK_FORMAT_B8G8R8A8_UNORM;
    result.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    return result;
  }

  for (const VkSurfaceFormatKHR& format : formats) {
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return format;
  }

  // At this point we don't have a prefered one, so we just return the first.
  return formats.front();
}

VkPresentModeKHR
ChooseSwapChainPresentMode(std::vector<VkPresentModeKHR>& present_modes) {
  // We search for mailbox (to do triple buffering).
  // FIFO is assured to exist.
  for (const VkPresentModeKHR& present_mode : present_modes) {
    if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
      return present_mode;
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& cap,
                                 Pair<uint32_t> screen_size) {
  // uint32_t::max means that the window manager lets us pick the extent.
  if (cap.currentExtent.width != UINT32_MAX)
    return cap.currentExtent;

  // We clamp our required extent to the bounds given by the GPU.
  VkExtent2D extent = {screen_size.x, screen_size.y};
  extent.width = Clamp(extent.width,
                       cap.minImageExtent.width, cap.maxImageExtent.width);
  extent.height = Clamp(extent.height,
                        cap.minImageExtent.height, cap.maxImageExtent.height);
  return extent;
}

}  // namespace

bool CreateSwapChain(Context* context, Pair<uint32_t> screen_size) {
  auto& capabilities = context->physical_device_info.swap_chain_capabilities;
  VkSurfaceFormatKHR format =
      ChooseSwapChainSurfaceFormat(capabilities.formats);
  VkPresentModeKHR present_mode =
      ChooseSwapChainPresentMode(capabilities.present_modes);

  VkExtent2D extent =
      ChooseSwapChainExtent(capabilities.capabilities, screen_size);

  uint32_t min_image_count = capabilities.capabilities.minImageCount;
  uint32_t max_image_count = capabilities.capabilities.maxImageCount;
  uint32_t image_count = Definitions::kNumFrames;
  ASSERT(image_count >= min_image_count);
  // We clamp the value if necessary. max == 0 means no limit in image count.
  if (max_image_count > 0 && image_count > max_image_count)
    image_count = max_image_count;

  VkSwapchainCreateInfoKHR create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = *context->surface;
  create_info.minImageCount = image_count;
  create_info.imageFormat = format.format;
  create_info.imageColorSpace = format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // If the graphics and present queues are different, we need to establish
  // sharing mode between them.
  auto& indices = context->physical_device_info.queue_family_indices;
  uint32_t indices_array[2] = {(uint32_t)indices.graphics,
                               (uint32_t)indices.present};
  if (indices.graphics != indices.present) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = ARRAY_SIZE(indices_array);
    create_info.pQueueFamilyIndices = indices_array;
  } else {
    // If they're the same, we establish no sharing.
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  // Leave the image as it is.
  create_info.preTransform = capabilities.capabilities.currentTransform;

  // We ingore the alpha for blending with the window system.
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;
  create_info.oldSwapchain = VK_NULL_HANDLE;

  VkSwapchainKHR swap_chain;
  VK_CHECK(vkCreateSwapchainKHR, *context->device, &create_info, nullptr,
                                 &swap_chain);

  context->swap_chain.Set(context, swap_chain);
  context->swap_chain_details.format = format;
  context->swap_chain_details.present_mode = present_mode;
  context->swap_chain_details.extent = extent;

  // Retrieve the images.
  uint32_t num_frames = Definitions::kNumFrames;
  VK_CHECK(vkGetSwapchainImagesKHR, *context->device, swap_chain,
                                    &num_frames, context->images);

  return true;
}

// CreateImageViews ------------------------------------------------------------

bool CreateImageViews(Context* context) {
  for (size_t i = 0; i < ARRAY_SIZE(context->images); i++) {
    CreateImageViewConfig config = {};
    config.image = context->images[i];
    config.format = context->swap_chain_details.format.format;
    config.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    auto image_view = CreateImageView(context, &config);
    if (!image_view.has_value())
      return false;

    context->image_views[i] = std::move(image_view);
  }

  return true;
}

// CreateCommandPool -----------------------------------------------------------

bool CreateCommandPool(Context* context) {
  VkCommandPoolCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  // Each command pool can only allocate commands for one particular queue
  // family.
  create_info.queueFamilyIndex =
      context->physical_device_info.queue_family_indices.graphics;
  create_info.flags = 0;  // Optional

  VkCommandPool command_pool;
  if (!VK_CALL(vkCreateCommandPool, *context->device, &create_info, nullptr,
                                    &command_pool)) {
    return false;
  }

  context->command_pool.Set(context, std::move(command_pool));
  return true;
}

// CreateDepthResources --------------------------------------------------------

namespace {

// Returns format undefined on error.
VkFormat FindDepthFormat(VkPhysicalDevice device,
                         VkImageTiling tiling, VkFormatFeatureFlags features,
                         const std::vector<VkFormat>& candidates) {
  for (const VkFormat& format : candidates) {
    // We get what the GPU supports for this format.

    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(device, format, &properties);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (properties.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (properties.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  return VK_FORMAT_UNDEFINED;
}

}  // namespace

bool CreateDepthResources(Context* context) {
  SCOPE_LOCATION();

  VkFormat depth_format =
      FindDepthFormat(context->physical_device,
                      VK_IMAGE_TILING_OPTIMAL,
                      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      {VK_FORMAT_D32_SFLOAT,
                       VK_FORMAT_D32_SFLOAT_S8_UINT,
                       VK_FORMAT_D24_UNORM_S8_UINT});

  if (depth_format == VK_FORMAT_UNDEFINED) {
    LOG(ERROR) << "Could not find a valid depth format.";
    return false;
  }

  // Allocate and image.
  AllocImageConfig alloc_image_config = {};
  VkImageCreateInfo& image_info = alloc_image_config.create_info;
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.format = depth_format;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.extent.width = context->swap_chain_details.extent.width;
  image_info.extent.height = context->swap_chain_details.extent.height;
  image_info.extent.depth = 1;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  alloc_image_config.memory_usage = MemoryUsage::kGPUOnly;

  CreateImageViewConfig image_view_config = {};
  /* image_view_config.image = *image.handle; */
  image_view_config.format = depth_format;
  image_view_config.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;

  TransitionImageLayoutConfig transition_config = {};
  transition_config.format = depth_format;
  transition_config.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  transition_config.new_layout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  CreateImageConfig create_image_config = {};
  create_image_config.alloc_config = std::move(alloc_image_config);
  create_image_config.view_config = std::move(image_view_config);
  create_image_config.transition_config = std::move(transition_config);
  auto created_image = CreateImage(context, &create_image_config);
  if (!created_image.valid())
    return false;

  context->depth_format = depth_format;
  context->depth_image = std::move(created_image.image);
  context->depth_image_view = std::move(created_image.image_view);

  return true;
}

}  // namespace vulkan
}  // namespace warhol
