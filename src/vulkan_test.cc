// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

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
    VulkanContext context;

    // Extensions
    res = GetSDLExtensions(window, &context);
    if (!res.ok()) {
      printf("Could not get SDL extensions: %s\n", res.err_msg().c_str());
      return 1;
    }

    // Validation layers
    if (kValidationLayersEnabled) {
      context.validation_layers.push_back(
          "VK_LAYER_LUNARG_standard_validation");
    }

    // We create the VkInstance.
    res = SetupSDLVulkanInstance(&context);
    if (!res.ok()) {
      printf("Error setting vulkan instance: %s\n", res.err_msg().c_str());
      return 1;
    }

    // Physical Devices.
    res = SetupVulkanPhysicalDevices(&context);
    if (!res.ok()) {
      printf("Error setting vulkan physical devices: %s\n",
             res.err_msg().c_str());
      return 1;
    }

    res = SetupVulkanLogicalDevices(&context);
    if (!res.ok()) {
      printf("Error setting vulkan logical devices: %s\n",
             res.err_msg().c_str());
      return 1;
    }
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

