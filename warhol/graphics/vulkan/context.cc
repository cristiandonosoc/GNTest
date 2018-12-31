// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/context.h"

#include <string.h>

#include <set>

#include "warhol/graphics/vulkan/utils.h"
#include "warhol/math/vec.h"
#include "warhol/utils/file.h"
#include "warhol/utils/log.h"
#include "warhol/utils/scope_trigger.h"

namespace warhol {
namespace vulkan {

// CreateContext & -------------------------------------------------------------

bool CreateContext(Context* context) {
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Warhol Vulkan Test";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName = "Warhol";
  app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo instance_create_info = {};
  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.pApplicationInfo = &app_info;

  instance_create_info.enabledExtensionCount = context->extensions.size();
  instance_create_info.ppEnabledExtensionNames = context->extensions.data();

  instance_create_info.enabledLayerCount = context->validation_layers.size();
  instance_create_info.ppEnabledLayerNames = context->validation_layers.data();

  VkInstance instance;
  if (auto res = vkCreateInstance(&instance_create_info, nullptr, &instance);
      res != VK_SUCCESS) {
    LOG(ERROR) << "Could not create instance: " << EnumToString(res);
    return false;
  }

  context->instance = instance;
  return true;
}

// ~Context --------------------------------------------------------------------

Context::~Context() {
  for (auto image_view : image_views)
    vkDestroyImageView(*device, image_view, nullptr);
  if (swap_chain)
    vkDestroySwapchainKHR(*device, *swap_chain, nullptr);
  if (device)
    vkDestroyDevice(*device, nullptr);
  if (surface)
    vkDestroySurfaceKHR(*instance, *surface, nullptr);

  if (debug_messenger)
    DestroyDebugUtilsMessengerEXT(*instance, *debug_messenger, nullptr);

  if (instance)
    vkDestroyInstance(*instance, nullptr);
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

  context->debug_messenger = debug_messenger;
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

  LOG(DEBUG) << "  Got device properties: ";
  for (const auto& format : details.formats)
    LOG(DEBUG) << "  - Format: " << EnumToString(format.format)
               << ", Color Space: " << EnumToString(format.colorSpace);

  VK_GET_PROPERTIES4(vkGetPhysicalDeviceSurfacePresentModesKHR,
                     device, surface, details.present_modes);
  for (const auto& present_mode : details.present_modes)
    LOG(DEBUG) << "  - Present Mode: " << EnumToString(present_mode);

  return details;
}

bool IsSuitableDevice(const VkPhysicalDevice& device,
                      const VkSurfaceKHR& surface,
                      const std::vector<const char*>& required_extensions,
                      PhysicalDeviceInfo* out) {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(device, &properties);

  LOG(INFO) << "Checking GPU: " << properties.deviceName;

  /* VkPhysicalDeviceFeatures features; */
  /* vkGetPhysicalDeviceFeatures(device, &features); */

  auto indices = FindQueueFamilyIndices(device, surface);
  if (indices.graphics == -1 || indices.present == -1)
    return false;

  if (!CheckPhysicalDeviceExtensions(device, required_extensions))
    return false;

  SwapChainCapabilities details = QuerySwapChainSupport(device, surface);
  if (details.formats.empty() || details.present_modes.empty())
    return false;

  out->queue_family_indices = indices;
  out->swap_chain_capabilities = details;
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
    PhysicalDeviceInfo device_info;
    if (IsSuitableDevice(device,
                         *context->surface,
                         context->device_extensions,
                         &device_info)) {
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
  context->device_info = device_info;
  return true;
}

// CreateLogicalDevice ---------------------------------------------------------

bool CreateLogicalDevice(Context* context) {
  // Setup the queues.
  /* auto indices = FindQueueFamilyIndices(context->physical_device, */
  /*                                       *context->surface); */

  // We create one per queue family required. If there are the same, we only
  // create one.
  auto& indices = context->device_info.queue_family_indices;
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
  create_info.pEnabledFeatures = &enabled_features;
  create_info.enabledExtensionCount = context->device_extensions.size();
  create_info.ppEnabledExtensionNames = context->device_extensions.data();

  VkDevice device;
  if (auto res = vkCreateDevice(
          context->physical_device, &create_info, nullptr, &device);
      res != VK_SUCCESS) {
    LOG(ERROR) << "Could not create logical device: " << EnumToString(res);
    return false;
  }

  context->device = device;
  vkGetDeviceQueue(device, indices.graphics, 0, &context->graphics_queue);
  vkGetDeviceQueue(device, indices.present, 0, &context->present_queue);
  return true;
}

// CreateSwapChain -------------------------------------------------------------

namespace {

VkSurfaceFormatKHR
ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
  // If vulkan only returns an undefined format, it means that it has no
  // preference and we can choose.
  if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
    return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

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
  // FIFO is assured to exist but apparently is not well supported so we should
  // prefer IMMEDIATE if available.
  VkPresentModeKHR best_mode = VK_PRESENT_MODE_FIFO_KHR;
  for (const VkPresentModeKHR& present_mode : present_modes) {
    if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return present_mode;
    } else if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      best_mode = present_mode;
    }
  }

  return best_mode;
}

VkExtent2D ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& cap,
                                 Pair<uint32_t> screen_size) {
  // uint32_t::max means that the window manager lets us pick the extent.
  if (cap.currentExtent.width != std::numeric_limits<uint32_t>::max())
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
  auto& capabilities = context->device_info.swap_chain_capabilities;
  VkSurfaceFormatKHR format =
      ChooseSwapChainSurfaceFormat(capabilities.formats);
  VkPresentModeKHR present_mode =
      ChooseSwapChainPresentMode(capabilities.present_modes);

  VkExtent2D extent =
      ChooseSwapChainExtent(capabilities.capabilities, screen_size);

  uint32_t min_image_count = capabilities.capabilities.minImageCount;
  uint32_t max_image_count = capabilities.capabilities.maxImageCount;
  uint32_t image_count = min_image_count + 1;
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
  auto& indices = context->device_info.queue_family_indices;
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

  create_info.preTransform = capabilities.capabilities.currentTransform;

  // We ingore the alpha for blending with the window system.
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;


  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;
  create_info.oldSwapchain = VK_NULL_HANDLE;

  VkSwapchainKHR swap_chain;
  if (auto res = vkCreateSwapchainKHR(
          *context->device, &create_info, nullptr, &swap_chain);
      res != VK_SUCCESS) {
    LOG(ERROR) << "Could not create swap chain: " << EnumToString(res);
    return false;
  }

  context->swap_chain = swap_chain;
  context->swap_chain_details.format = format;
  context->swap_chain_details.present_mode = present_mode;
  context->swap_chain_details.extent = extent;

  // Retrieve the images.
  VK_GET_PROPERTIES4(vkGetSwapchainImagesKHR,
                     *context->device, swap_chain, context->images);
  return true;
}

// CreateImageViews ------------------------------------------------------------

bool CreateImageViews(Context* context) {
  context->image_views.clear();
  for (size_t i = 0; i < context->images.size(); i++) {
    VkImageViewCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = context->images[i];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = context->swap_chain_details.format.format;

    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // This is an image (color texture).
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    VkImageView image_view;
    if (auto res = vkCreateImageView(
            *context->device, &create_info, nullptr, &image_view);
        res != VK_SUCCESS) {
      LOG(ERROR) << "Could not create image view: " << EnumToString(res);
      return false;
    }
    context->image_views.push_back(std::move(image_view));
  }

  return true;
}

// CreateGraphicsPipeline ------------------------------------------------------

namespace {

VkShaderModule CreateShaderModule(const VkDevice& device,
                                  const std::vector<char>& data) {
  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = data.size();
  create_info.pCode = (const uint32_t*)(data.data());

  VkShaderModule shader;
  if (auto res = vkCreateShaderModule(device, &create_info, nullptr, &shader);
      res != VK_SUCCESS) {
    LOG(ERROR) << "Could not create shader module: " << EnumToString(res);
    return VK_NULL_HANDLE;
  }
  return shader;
}

}  // namespace

bool CreateGraphicsPipeline(Context* context,
                            const std::string& vert_path,
                            const std::string& frag_path) {
  std::vector<char> vert_data, frag_data;
  if (!ReadWholeFile(vert_path, &vert_data, false) ||
      !ReadWholeFile(frag_path, &frag_data, false)) {
    return false;
  }

  LOG(DEBUG) << "Vert data size: " << vert_data.size();
  LOG(DEBUG) << "Frag data size: " << frag_data.size();

  VkShaderModule vert_module = CreateShaderModule(*context->device, vert_data);
  ScopeTrigger vert_trigger([&vert_module, &device = *context->device]() {
    if (vert_module != VK_NULL_HANDLE)
      vkDestroyShaderModule(device, vert_module, nullptr);
  });

  VkShaderModule frag_module = CreateShaderModule(*context->device, frag_data);
  ScopeTrigger frag_trigger([&frag_module, &device = *context->device]() {
    if (frag_module != VK_NULL_HANDLE)
      vkDestroyShaderModule(device, frag_module, nullptr);
  });

  if (vert_module == VK_NULL_HANDLE || frag_module == VK_NULL_HANDLE)
    return false;

  VkPipelineShaderStageCreateInfo vert_create_info = {};
  vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_create_info.module = vert_module;
  vert_create_info.pName = "main";

  VkPipelineShaderStageCreateInfo frag_create_info = {};
  frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_create_info.module = frag_module;
  frag_create_info.pName = "main";

  /* VkPipelineShaderStageCreateInfo shader_stages[] = {vert_create_info, */
  /*                                                    frag_create_info}; */

  LOG(WARNING) << "Not implemented.";
  return false;
}

}  // namespace vulkan
}  // namespace warhol
