// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/graphics/vulkan/context.h"

#include <string.h>

#include <set>

#include "warhol/graphics/vulkan/utils.h"
#include "warhol/utils/log.h"

namespace warhol {
namespace vulkan {

// CreateContext & -------------------------------------------------------------

bool CreateContext(const std::vector<const char*>& extensions,
                   const std::vector<const char*>& layers,
                   Context* out) {
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

  instance_create_info.enabledExtensionCount = extensions.size();
  instance_create_info.ppEnabledExtensionNames = extensions.data();

  instance_create_info.enabledLayerCount = layers.size();
  instance_create_info.ppEnabledLayerNames = layers.data();

  VkInstance instance;
  if (auto res = vkCreateInstance(&instance_create_info, nullptr, &instance);
      res != VK_SUCCESS) {
    LOG(ERROR) << "Could not create instance: " << EnumToString(res);
    return false;
  }

  out->instance = instance;
  return true;
}

// ~Context --------------------------------------------------------------------

Context::~Context() {
  if (debug_messenger)
    DestroyDebugUtilsMessengerEXT(*instance, *debug_messenger, nullptr);

  if (device)
    vkDestroyDevice(*device, nullptr);
  if (surface)
    vkDestroySurfaceKHR(*instance, *surface, nullptr);
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

struct QueueFamilyIndices {
  int graphics = -1;
  int present = -1;
};

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

bool IsSuitableDevice(const VkPhysicalDevice& device,
                      const VkSurfaceKHR& surface) {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(device, &properties);

  LOG(INFO) << "Checking GPU: " << properties.deviceName;

  /* VkPhysicalDeviceFeatures features; */
  /* vkGetPhysicalDeviceFeatures(device, &features); */

  auto indices = FindQueueFamilyIndices(device, surface);
  if (indices.graphics == -1)
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

  std::vector<VkPhysicalDevice> suitable_devices;
  for (const VkPhysicalDevice& device : devices) {
    if (IsSuitableDevice(device, *context->surface)) {
      suitable_devices.push_back(device);
      LOG(INFO) << "- Suitable!";
    }
  }

  if (suitable_devices.empty()) {
    LOG(ERROR) << "Could not find a suitable physical device.";
    return false;
  }

  // For now we choose the first device.
  context->physical_device = suitable_devices.front();
  return true;
}

// CreateLogicalDevice ---------------------------------------------------------

bool CreateLogicalDevice(Context* context) {
  // Setup the queues.
  auto indices = FindQueueFamilyIndices(context->physical_device,
                                        *context->surface);

  // We create one per queue family required. If there are the same, we only
  // create one.
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

}  // namespace vulkan
}  // namespace warhol
