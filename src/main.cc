// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

#define SDL_MAIN_HANDLED
#include <SDL2/sdl.h>

#include "vulkan_context.h"

#include "utils/file.h"

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

  warhol::VulkanContext vulkan_context = {};
  warhol::Status res = InitVulkanContext(window, &vulkan_context);
  if (!res.ok()) {
    printf("ERROR INITIALIZING VULKAN: %s\n", res.err_msg().data());
    return 1;
  }

  printf("Correctly initialized vulkan\n");
  return 0;
}
