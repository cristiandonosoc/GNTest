// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>


#define SDL_MAIN_HANDLED
#include <SDL2/sdl.h>
#include <vulkan/vulkan.h>

int main() {

  uint32_t extension_count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

  printf("Hello Vulkan!\n");
  printf("Extensions supported: %d\n", extension_count);

  // Setup SDL2
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    printf("Error loading SDL: %s\n", SDL_GetError());
    return 1;
  }

  // Data about displays
  printf("Information from SDL\n");
  printf("Amount of displays: %d\n", SDL_GetNumVideoDisplays());

  return 0;
}

