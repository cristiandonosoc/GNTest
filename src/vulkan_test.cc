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

#ifdef NDEBUG
bool kValidationLayersEnabled = false;
#else
bool kValidationLayersEnabled = true;
#endif

using namespace warhol;

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

Status
SetupVulkanInstance(SDL_Window* window, VkInstance* instance) {
  // Vulkan application info.
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Vulkan Test";
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName = "Warhol";
  app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.apiVersion = VK_API_VERSION_1_1;

  // Setup extensions.
  uint32_t extension_count;
  if(!SDL_Vulkan_GetInstanceExtensions(window, &extension_count, NULL)) {
    return Status("Error getting vulkan extensions: %s\n", SDL_GetError());
  }
  std::vector<const char*> extensions(extension_count);
  SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extensions.data());

  // Debug output.
  printf("SDL extensions (count: %d):\n", extension_count);
  for (size_t i = 0; i < extension_count; i++)
    printf("- %s\n", extensions[i]);
  printf("\n");

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = extension_count;
  create_info.ppEnabledExtensionNames = extensions.data();
  create_info.enabledLayerCount = 0;

  // Setup validation layers.
  std::vector<const char*> validation_layers;
  if (kValidationLayersEnabled) {
    validation_layers.push_back("VK_LAYER_LUNARG_standard_validation");
    if (!CheckVulkanValidationLayers(validation_layers))
      return Status("Not all requested validation layers are available");

    create_info.enabledLayerCount = (uint32_t)validation_layers.size();
    create_info.ppEnabledLayerNames = validation_layers.data();
  }


  // Finally create the VkInstance.
  VkResult result = vkCreateInstance(&create_info, nullptr, instance);
  if (result != VK_SUCCESS)
    return Status("Error creating vulkan instance\n");

  return Status();
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

  VkInstance instance;
  Status res = SetupVulkanInstance(window, &instance);
  if (!res.ok()) {
    printf("Error creating window: %s\n", res.err_msg().c_str());
    return 1;
  }

  vkDestroyInstance(instance, nullptr);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

