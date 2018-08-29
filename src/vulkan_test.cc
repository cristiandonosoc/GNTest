// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include <memory>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL2/sdl.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "utils/status.h"
#include "vulkan_context.h"
#include "vulkan_utils.h"

#ifdef NDEBUG
bool kDebug = false;
#else
bool kDebug = true;
#endif

using namespace warhol;

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
    Status res;
    InstanceContext instance;
    SelectedContext context;
    context.instance = &instance;

    // Extensions
    res = GetSDLExtensions(window, &instance);
    if (!res.ok()) {
      printf("Could not get SDL extensions: %s\n", res.err_msg().c_str());
      return 1;
    }

    if (kDebug) {
      // Add debug extensions.
      instance.extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

      // Add validation layers
      instance.validation_layers.push_back(
          "VK_LAYER_LUNARG_standard_validation");
    }

    // ******** INSTANCE ********

    // We create the VkInstance.
    res = SetupSDLVulkanInstance(&instance);
    if (!res.ok()) {
      printf("Error setting vulkan instance: %s\n", res.err_msg().c_str());
      return 1;
    }

    printf("INSTANCE\n"); fflush(stdout);
    // ******** PHYSICAL_DEVICE ********

    // Physical Devices. Creates a surface.
    res = SetupVulkanPhysicalDevices(window, &instance);
    if (!res.ok()) {
      printf("Error setting vulkan physical devices: %s\n",
             res.err_msg().c_str());
      return 1;
    }

    std::vector<const char*> physical_device_extensions;
    physical_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    auto* suitable_device =
        FindSuitablePhysicalDevice(&instance, physical_device_extensions);
    if (!suitable_device) {
      printf("Found no suitable device\n");
      return 1;
    }
    context.physical_device = suitable_device;

    printf("PHYSICAL DEVICE\n"); fflush(stdout);
    // ******** LOGICAL_DEVICE ********

    // TODO: This function should output a created logical device and we
    //       should add it to the physical device.
    res = SetupVulkanLogicalDevices(&instance, context.physical_device,
                                    physical_device_extensions);
    if (!res.ok()) {
      printf("Error setting vulkan logical devices: %s\n",
             res.err_msg().c_str());
      return 1;
    }

    // For now select the first logical device as selected
    context.logical_device =
        context.physical_device->logical_devices.back().get();

    printf("LOGICAL DEVICE\n"); fflush(stdout);
    // ******** SWAP_CHAIN ********

    res = SetupSwapChain(context.physical_device, context.logical_device);
    if (!res.ok()) {
      printf("Error setting up swapchain: %s\n", res.err_msg().c_str());
      return 1;
    }
    context.swap_chain = context.logical_device->swap_chain.get();

    printf("SWAP CHAIN\n"); fflush(stdout);
    // ******** IMAGE_VIEWS ********

    res = CreateImageViews(context.swap_chain);
    if (!res.ok()) {
      printf("Error setting up image views: %s\n", res.err_msg().c_str());
      return 1;
    }

    printf("IMAGE VIEWS\n"); fflush(stdout);
  }

  printf("Logical device set\n");
  fflush(stdout);

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

