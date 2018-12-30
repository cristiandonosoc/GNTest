// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <iostream>

#include <vulkan/vulkan.h>
#include <warhol/utils/log.h>
#include <warhol/graphics/vulkan/context.h>
#include <warhol/graphics/vulkan/utils.h>
#include <warhol/sdl2/sdl_context.h>
#include <SDL2/SDL_vulkan.h>

using namespace warhol;

namespace {

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

int main() {
  SDLContext sdl_context;
  if (!sdl_context.InitVulkan(0))
    return 1;

  std::vector<const char*> extensions = {};
  VK_GET_PROPERTIES(SDL_Vulkan_GetInstanceExtensions,
                    sdl_context.get_window(),
                    extensions);
#ifndef NDEBUG
  vulkan::AddDebugExtensions(&extensions);
#endif
  if (!vulkan::CheckExtensions(extensions))
    return 1;

  std::vector<const char*> layers = {};
#ifndef NDEBUG
  layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
  if (!vulkan::CheckValidationLayers(layers))
    return 1;

  vulkan::Context context;
  if (!vulkan::CreateContext(extensions, layers, &context))
    return 1;
  LOG(INFO) << "Created context.";

  if (!vulkan::SetupDebugCall(&context, VulkanDebugCall))
    return 1;
  LOG(INFO) << "Set debug callback.";

  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(
          sdl_context.get_window(), *context.instance, &surface)) {
    LOG(ERROR) << "Could not create surface: " << SDL_GetError();
  }
  context.surface = surface;
  LOG(INFO) << "Created a surface.";

  if (!vulkan::PickPhysicalDevice(&context))
    return 1;
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(context.physical_device, &properties);
  LOG(INFO) << "Picked physical device: " << properties.deviceName;

  if (!vulkan::CreateLogicalDevice(&context))
    return 1;
  LOG(INFO) << "Created a logical device.";
}
